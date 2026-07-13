#include "GeometryMeshCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    bool GeometryMeshCullingPass::ShouldExecute(const RenderContext& context) const
    {
		return context.scene->GetSettings()->culling.geometryCullingDevice == CullingDeviceType::GPU;
    }

    void GeometryMeshCullingPass::Initialize() {
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("GeometryMeshCullingProgram", {
            ShaderNames::GeometryMeshCullingComp
            }, config);
    }

    void GeometryMeshCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        uint32_t totalModels = modelPool ? static_cast<uint32_t>(modelPool->Size()) : 0;

        if (totalModels == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ModelMeshCullingPC> pc;
		pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void GeometryMeshCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        //Using prevous frame's depth pyramid!
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
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

    void GeometryMeshCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (!_shouldDispatch) return;

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

		auto countBuffer = drawData->Models.computeCountBuffer.GetHandle(fIdx);
        vkCmdDispatchIndirect(context.cmd, countBuffer, 0);

        Vk::BufferBarrierInfo drawCmdBarrier{};
        drawCmdBarrier.buffer = drawData->Models.indirectBuffer.GetHandle(fIdx);
        drawCmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawCmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        drawCmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawCmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, drawCmdBarrier);

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->Models.instanceBuffer.GetHandle(fIdx);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);
    }
}