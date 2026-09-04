#include "ShadowFinalTransitionPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn {
    void ShadowFinalTransitionPass::PrepareFrame(const RenderContext& context) {
        if (auto activeScene = context.scene) {
            if (auto drawData = activeScene->GetSceneDrawData()) {
                uint32_t fIdx = context.frameIndex;

                std::vector<Vk::Image*> shadowImages = {
                    drawData->DirectionLightShadow.shadowAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.staticShadowAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.staticShadowColorAtlas[fIdx].get(),
                    drawData->SpotLightShadow.shadowAtlas[fIdx].get(),
                    drawData->SpotLightShadow.shadowColorAtlas[fIdx].get(),
                    drawData->PointLightShadow.shadowAtlas[fIdx].get(),
                    drawData->PointLightShadow.shadowColorAtlas[fIdx].get()
                };

                for (auto* img : shadowImages) {
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