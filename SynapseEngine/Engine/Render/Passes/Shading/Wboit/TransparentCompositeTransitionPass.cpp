#include "TransparentCompositeTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    bool TransparentCompositeTransitionPass::ShouldExecute(const RenderContext& context) const
    {
        return !context.scene->GetSettings()->enableDebugVisibility;
    }

    void TransparentCompositeTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        std::vector<std::string> wboitTargets = {
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal
        };

        for (const auto& target : wboitTargets) {
            if (auto img = group->GetImage(target)) {
                if (img->GetLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                    _imageTransitions.push_back({
                        .image = img,
                        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                        .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
                        .discardContent = false
                        });
                }
            }
        }

        if (auto mainImg = group->GetImage(RenderTargetNames::Main)) {
            if (mainImg->GetLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = mainImg,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .discardContent = false
                    });        
            }
        }
    }
}