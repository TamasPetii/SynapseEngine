#include "SpotLightCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SpotLightCullingPC.glsl"

    bool SpotLightCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->culling.spotLightCullingDevice == CullingDeviceType::GPU;
    }

    void SpotLightCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("SpotLightCullingProgram", {
            ShaderNames::SpotLightCulling
            }, { .useDescriptorBuffers = false });
    }

    void SpotLightCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto registry = scene->GetRegistry();
        auto spotLightPool = registry->GetPool<SpotLightComponent>();
        _totalLightsToTest = spotLightPool ? static_cast<uint32_t>(spotLightPool->Size()) : 0;

        if (_totalLightsToTest == 0) return;

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<SpotLightCullingPC> pc;
		pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto prevRtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
        auto depthPyramid = prevRtGroup->GetImage(RenderTargetNames::DepthPyramid);
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

    void SpotLightCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (_totalLightsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalLightsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo cmdBarrier{};
        cmdBarrier.buffer = drawData->SpotLights.indirectBuffer.GetHandle(fIdx);
        cmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        cmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cmdBarrier);

        Vk::BufferBarrierInfo visibleBarrier{};
        visibleBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightVisibleData, fIdx).buffer->Handle();
        visibleBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        visibleBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleBarrier);

        Vk::BufferBarrierInfo colliderDataBarrier{};
        colliderDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightColliderData, fIdx).buffer->Handle();
        colliderDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        colliderDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        colliderDataBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        colliderDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, colliderDataBarrier);

        Vk::BufferBarrierInfo shadowCountBarrier{};
        shadowCountBarrier.buffer = drawData->SpotLightShadow.visibleCountDispatchBuffer.GetHandle(fIdx);
        shadowCountBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        shadowCountBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        shadowCountBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        shadowCountBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, shadowCountBarrier);

        Vk::BufferBarrierInfo sortKeyBarrier{};
        sortKeyBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightShadowAtlasSortKeyBuffer, fIdx).buffer->Handle();
        sortKeyBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortKeyBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortKeyBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortKeyBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortKeyBarrier);

        Vk::BufferBarrierInfo sortValueBarrier{};
        sortValueBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightShadowAtlasSortValueBuffer, fIdx).buffer->Handle();
        sortValueBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortValueBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortValueBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortValueBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortValueBarrier);
    }
}