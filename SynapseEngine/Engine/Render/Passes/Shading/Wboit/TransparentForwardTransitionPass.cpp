#include "TransparentForwardTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    bool TransparentForwardTransitionPass::ShouldExecute(const RenderContext& context) const
    {
        return !context.scene->GetSettings()->debug.enableDebugVisibility;
    }

    void TransparentForwardTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        std::vector<std::string> colorTargets = {
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal,
            RenderTargetNames::TransparentAccumMSAA,
            RenderTargetNames::TransparentRevealMSAA
        };

        for (const auto& name : colorTargets)
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

        std::vector<std::string> depthTargets = {
            RenderTargetNames::OpaqueDepth,
            RenderTargetNames::OpaqueDepthMSAA
        };

        for (const auto& name : depthTargets)
        {
            if (auto depthImg = group->GetImage(name)) {
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
}