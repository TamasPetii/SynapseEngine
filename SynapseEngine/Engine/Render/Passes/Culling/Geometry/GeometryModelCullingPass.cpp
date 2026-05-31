#include "GeometryModelCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    bool GeometryModelCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableGeometryGpuCulling;
    }

    void GeometryModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("GeometryModelCullingProgram", {
            ShaderNames::GeometryModelCullingComp
            }, config);
    }

    void GeometryModelCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();

        if (!transformPool || transformPool->Size() == 0) {
            _totalModelsToTest = 0;
            return;
        }

        _totalModelsToTest = static_cast<uint32_t>(transformPool->Size());

        if (scene->GetSettings()->enableStaticBvhCulling || scene->GetSettings()->enableMortonBvhCulling) {
            uint32_t staticCount = static_cast<uint32_t>(transformPool->GetStorage().GetStaticEntities().size());
            _totalModelsToTest -= staticCount;
        }

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        ModelMeshCullingPC pc{};
        pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, isGpu);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ModelMeshCullingPC), &pc);
    }

    void GeometryModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        //Using prevous frame's depth pyramid!
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void GeometryModelCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (_totalModelsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;
		auto isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalModelsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->Models.computeCountBuffer.GetHandle(fIdx, isGpu);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = compManager->GetComponentBuffer(BufferNames::ModelVisibleData, fIdx).buffer->Handle();
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);
    }
}