#include "SpotLightShadowMeshCullingPass.h"
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
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SpotLightShadowCullingPC.glsl"

    bool SpotLightShadowMeshCullingPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        
        return context.scene->GetSettings()->culling.spotLightShadowCullingDevice == CullingDeviceType::GPU 
            && pool && pool->Size() > 0;
    }

    void SpotLightShadowMeshCullingPass::Initialize() {
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("SpotLightShadowMeshCullingProgram", {
            ShaderNames::SpotLightShadowMeshCullingComp
            }, config);
    }

    void SpotLightShadowMeshCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto modelPool = scene->GetRegistry()->GetPool<ModelComponent>();
        uint32_t totalModels = modelPool ? static_cast<uint32_t>(modelPool->Size()) : 0;

        if (totalModels == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<SpotLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowMeshCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto depthPyramid = context.scene->GetSceneDrawData()->SpotLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        //pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void SpotLightShadowMeshCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (!_shouldDispatch) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        Vk::BufferBarrierInfo dispatchBarrier{};
        dispatchBarrier.buffer = drawData->SpotLightShadow.modelDispatchBuffer.GetHandle(fIdx);
        dispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dispatchBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        dispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        dispatchBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, dispatchBarrier);

        Vk::BufferBarrierInfo visibleIndexBarrier{};
        visibleIndexBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightShadowModelVisibleData, fIdx).buffer->Handle();
        visibleIndexBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleIndexBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleIndexBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleIndexBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleIndexBarrier);

        Vk::BufferBarrierInfo meshCountBarrier{};
        meshCountBarrier.buffer = drawData->SpotLightShadow.visibleMeshCountDispatchBuffer.GetHandle(fIdx);
        meshCountBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        meshCountBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        meshCountBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        meshCountBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, meshCountBarrier);

        auto countBuffer = drawData->SpotLightShadow.modelDispatchBuffer.GetHandle(fIdx);
        vkCmdDispatchIndirect(context.cmd, countBuffer, 0);
    }
}