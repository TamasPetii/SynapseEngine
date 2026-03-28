#include "DepthCopyPass.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/System/RenderSystem.h"

namespace Syn {
    void DepthCopyPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        if (!group) return;

        auto srcDepth = group->GetImage(RenderTargetNames::Depth);
        auto dstDepth = group->GetImage(RenderTargetNames::EditorPickingDepth);
        if (!srcDepth || !dstDepth) return;


        _imageTransitions.push_back({
            .image = srcDepth,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = dstDepth,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .discardContent = true
            });
    }

    void DepthCopyPass::Transfer(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        if (!group) return;

        auto srcDepth = group->GetImage(RenderTargetNames::Depth);
        auto dstDepth = group->GetImage(RenderTargetNames::EditorPickingDepth);
        if (!srcDepth || !dstDepth) return;

        Vk::ImageCopyInfo copyInfo{};
        copyInfo.srcImage = srcDepth->Handle();
        copyInfo.dstImage = dstDepth->Handle();
        copyInfo.srcSize = { group->GetWidth(), group->GetHeight(), 1 };
        copyInfo.dstSize = { group->GetWidth(), group->GetHeight(), 1 };
        copyInfo.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        Vk::ImageUtils::CopyImage(context.cmd, copyInfo);

        srcDepth->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            false
        );

        dstDepth->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            false
        );
    }
}