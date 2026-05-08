#include "PresentationTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    void PresentationTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        std::vector<std::string> gBufferTargets = {
            RenderTargetNames::Main,
            RenderTargetNames::ColorMetallic,
            RenderTargetNames::NormalRoughness,
            RenderTargetNames::EmissiveAo,
            RenderTargetNames::TransparentAccum,
			RenderTargetNames::TransparentReveal
        };

        for (const auto& target : gBufferTargets) {
            if (auto img = group->GetImage(target)) {
                if (img->GetLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
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
    }
}