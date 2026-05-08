#include "TransparentForwardTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {

    void TransparentForwardTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        std::vector<std::string> targets = {
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal
        };

        for (const auto& name : targets)
        {
            if (auto img = group->GetImage(name)) {
                if (img->GetLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
					_imageTransitions.push_back({
						.image = img,
						.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
						.dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
						.discardContent = false
						});
                }
            }
        }

        if (auto depthImg = group->GetImage(RenderTargetNames::OpaqueDepth)) {
            if (depthImg->GetLayout() != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = depthImg,
                    .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    .discardContent = false
                    });
            }
        }


    }
}