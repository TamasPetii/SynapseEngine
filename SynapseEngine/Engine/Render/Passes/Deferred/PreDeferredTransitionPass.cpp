#include "PreDeferredTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    void PreDeferredTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        if (!group) return;

        std::vector<std::string> gBufferTargets = {
            RenderTargetNames::ColorMetallic,
            RenderTargetNames::NormalRoughness,
            RenderTargetNames::EmissiveAo
        };

        for (const auto& target : gBufferTargets) {
            if (auto img = group->GetImage(target)) {
                _imageTransitions.push_back({
                    .image = img,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
                    .discardContent = false
                    });
            }
        }

        if (auto depthImg = group->GetImage(RenderTargetNames::Depth)) {
            _imageTransitions.push_back({
                .image = depthImg,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
                .discardContent = false
                });
        }

        _renderInfo = std::nullopt;
    }
}