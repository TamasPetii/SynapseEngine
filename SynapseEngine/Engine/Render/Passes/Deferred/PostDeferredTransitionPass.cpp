#include "PostDeferredTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    void PostDeferredTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        if (!group) return;

        if (auto depthImg = group->GetImage(RenderTargetNames::Depth)) {
            _imageTransitions.push_back({
                .image = depthImg,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .discardContent = false
                });
        }

        _renderInfo = std::nullopt;
    }
}