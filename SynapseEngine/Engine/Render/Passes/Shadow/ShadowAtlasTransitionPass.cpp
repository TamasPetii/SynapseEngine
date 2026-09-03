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

#include "ShadowAtlasTransitionPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn 
{
    void ShadowAtlasTransitionPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto fIdx = context.frameIndex;

        std::vector<Vk::Image*> shadowImages = {
            drawData->DirectionLightShadow.shadowAtlas[fIdx].get(),
            drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get(),
            drawData->PointLightShadow.shadowAtlas[fIdx].get(),
            drawData->PointLightShadow.shadowColorAtlas[fIdx].get(),
            drawData->SpotLightShadow.shadowAtlas[fIdx].get(),
            drawData->SpotLightShadow.shadowColorAtlas[fIdx].get()
        };

        for (auto* img : shadowImages) {
            if (img) {
                _imageTransitions.push_back({
                    .image = img,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
                    .discardContent = false
                    });
            }
        }
    }
}