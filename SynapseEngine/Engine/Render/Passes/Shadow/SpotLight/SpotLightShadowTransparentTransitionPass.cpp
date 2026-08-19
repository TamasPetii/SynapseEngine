#include "SpotLightShadowTransparentTransitionPass.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    void SpotLightShadowTransparentTransitionPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto fIdx = context.frameIndex;
        auto spotShadowAtlas = drawData->SpotLightShadow.shadowAtlas[fIdx].get();

        if (spotShadowAtlas) {
            _imageTransitions.push_back({
                .image = spotShadowAtlas,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .discardContent = false
                });
        }
    }
}