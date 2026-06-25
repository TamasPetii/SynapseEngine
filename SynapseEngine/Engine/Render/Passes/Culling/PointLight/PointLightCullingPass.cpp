#include "PointLightCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/PointLightCullingPC.glsl"

    bool PointLightCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->culling.pointLightCullingDevice == CullingDeviceType::GPU;
    }

    void PointLightCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("PointLightCullingProgram", {
            ShaderNames::PointLightCulling
            }, config);
    }

    void PointLightCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto registry = scene->GetRegistry();
        auto pointLightPool = registry->GetPool<PointLightComponent>();
        _totalLightsToTest = pointLightPool ? static_cast<uint32_t>(pointLightPool->Size()) : 0;

        if (_totalLightsToTest == 0) return;

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<PointLightCullingPC> pc;
		pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto prevRtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
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

    void PointLightCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (_totalLightsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalLightsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo cmdBarrier{};
        cmdBarrier.buffer = drawData->PointLights.indirectBuffer.GetHandle(fIdx);
        cmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        cmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cmdBarrier);

        Vk::BufferBarrierInfo visibleDataBarrier{};
        visibleDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightVisibleData, fIdx).buffer->Handle();
        visibleDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleDataBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        visibleDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleDataBarrier);

        Vk::BufferBarrierInfo colliderDataBarrier{};
        colliderDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightColliderData, fIdx).buffer->Handle();
        colliderDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        colliderDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        colliderDataBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        colliderDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, colliderDataBarrier);

        Vk::BufferBarrierInfo sortKeyBarrier{};
        sortKeyBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightShadowAtlasSortKeyBuffer, fIdx).buffer->Handle();
        sortKeyBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortKeyBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortKeyBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortKeyBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortKeyBarrier);

        Vk::BufferBarrierInfo sortValueBarrier{};
        sortValueBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightShadowAtlasSortValueBuffer, fIdx).buffer->Handle();
        sortValueBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortValueBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortValueBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortValueBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortValueBarrier);
    }
}