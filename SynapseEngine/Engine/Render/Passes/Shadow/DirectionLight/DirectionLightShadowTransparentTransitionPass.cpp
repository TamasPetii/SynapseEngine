#include "DirectionLightShadowTransparentTransitionPass.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    void DirectionLightShadowTransparentTransitionPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto fIdx = context.frameIndex;
        auto dirShadowAtlas = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();

        if (dirShadowAtlas) {
            _imageTransitions.push_back({
                .image = dirShadowAtlas,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .discardContent = false
                });
        }
    }
}