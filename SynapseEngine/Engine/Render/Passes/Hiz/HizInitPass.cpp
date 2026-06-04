#include "HizInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn {

    void HizInitPass::PrepareFrame(const RenderContext& context) {
        //Using prevous frame's depth pyramid!
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
        auto drawData = context.scene->GetSceneDrawData();

        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        if (depthPyramid && depthPyramid->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED) 
        {
            depthPyramid->TransitionLayout(
                context.cmd,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_CLEAR_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                true
            );

            Vk::ImageClearColorInfo clearInfo{};
            clearInfo.image = depthPyramid->Handle();
            clearInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            clearInfo.clearColor = { {1.0f, 1.0f, 0.0f, 0.0f} };
            clearInfo.levelCount = depthPyramid->GetConfig().generateMipMaps ? Vk::ImageUtils::CalculateMipLevels(depthPyramid->GetConfig().width, depthPyramid->GetConfig().height) : 1;

            Vk::ImageUtils::ClearColorImage(context.cmd, clearInfo);

            depthPyramid->TransitionLayout(
                context.cmd,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                false
            );
        }

        auto& shadowPyramid = drawData->DirectionLightShadow.shadowDepthPyramid[context.frameIndex];
        if (shadowPyramid && shadowPyramid->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
        {
            shadowPyramid->TransitionLayout(
                context.cmd,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_CLEAR_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                true
            );

            Vk::ImageClearColorInfo clearInfo{};
            clearInfo.image = shadowPyramid->Handle();
            clearInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            clearInfo.clearColor = { {1.0f, 1.0f, 0.0f, 0.0f} };
            clearInfo.levelCount = shadowPyramid->GetConfig().mipLevels;

            Vk::ImageUtils::ClearColorImage(context.cmd, clearInfo);

            shadowPyramid->TransitionLayout(
                context.cmd,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                false
            );
        }
    }
}