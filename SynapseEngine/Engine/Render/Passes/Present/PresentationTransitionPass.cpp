// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "PresentationTransitionPass.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn {
    void PresentationTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        std::vector<std::string> guiTargets = {
            RenderTargetNames::Main,
            RenderTargetNames::ColorMetallic,
            RenderTargetNames::NormalRoughness,
            RenderTargetNames::EmissiveAo,
            RenderTargetNames::SsaoAo,
            RenderTargetNames::EntityIndex,
            RenderTargetNames::DepthPyramid,
            RenderTargetNames::Bloom,
            RenderTargetNames::OpaqueDepth,
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal
        };

        for (const auto& target : guiTargets) {
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

        if (auto activeScene = context.scene) {
            if (auto drawData = activeScene->GetSceneDrawData()) {
                uint32_t fIdx = context.frameIndex;
                std::vector<Vk::Image*> shadowImages = {
                    drawData->DirectionLightShadow.shadowAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.staticShadowAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.staticShadowColorAtlas[fIdx].get(),
                    drawData->DirectionLightShadow.shadowDepthPyramid[fIdx].get(),
                    drawData->SpotLightShadow.shadowAtlas[fIdx].get(),
                    drawData->SpotLightShadow.shadowColorAtlas[fIdx].get(),
                    drawData->SpotLightShadow.shadowDepthPyramid[fIdx].get(),
                    drawData->PointLightShadow.shadowAtlas[fIdx].get(),
                    drawData->PointLightShadow.shadowColorAtlas[fIdx].get(),
                    drawData->PointLightShadow.shadowDepthPyramid[fIdx].get()
                };

                for (auto* img : shadowImages) {
                    if (img && img->GetLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
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
}