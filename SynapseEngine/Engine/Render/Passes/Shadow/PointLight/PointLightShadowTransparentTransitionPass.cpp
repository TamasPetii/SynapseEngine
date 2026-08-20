#include "PointLightShadowTransparentTransitionPass.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    void PointLightShadowTransparentTransitionPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto fIdx = context.frameIndex;
        auto pointShadowAtlas = drawData->PointLightShadow.shadowAtlas[fIdx].get();

        if (pointShadowAtlas) {
            _imageTransitions.push_back({
                .image = pointShadowAtlas,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .discardContent = false
                });
        }
    }
}