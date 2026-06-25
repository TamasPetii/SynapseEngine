#include "ShadowAtlasTransitionPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn {

    void ShadowAtlasTransitionPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto fIdx = context.frameIndex;
        auto dirShadowAtlas = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();
        auto pointShadowAtlas = drawData->PointLightShadow.shadowAtlas[fIdx].get();
        auto spotShadowAtlas = drawData->SpotLightShadow.shadowAtlas[fIdx].get();

        _imageTransitions.push_back({
            .image = dirShadowAtlas,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
        });

        _imageTransitions.push_back({
            .image = pointShadowAtlas,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = spotShadowAtlas,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });
    }
}