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

#include "DirectionLightShadowAtlasRestorePass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn
{
    bool DirectionLightShadowAtlasRestorePass::ShouldExecute(const RenderContext& context) const {
        auto drawData = context.scene->GetSceneDrawData();
        return drawData && drawData->DirectionLightShadow.visibleLightCount > 0;
    }

    void DirectionLightShadowAtlasRestorePass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto registry = context.scene->GetRegistry();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();

        uint32_t activeLights = drawData->DirectionLightShadow.visibleLightCount;
        if (activeLights == 0 || !shadowPool) return;

        std::vector<VkImageCopy> depthRegions;
        std::vector<VkImageCopy> colorRegions;
        depthRegions.reserve(activeLights * 4);
        colorRegions.reserve(activeLights * 4);

        for (uint32_t i = 0; i < activeLights; ++i) {
            EntityID entity = drawData->DirectionLightShadow.visibleLights[i];
            const auto& shadowComp = shadowPool->Get(entity);

            for (int c = 0; c < 4; ++c) {
                float uvW = shadowComp.cascadeAtlasRects[c].z;

                if (uvW > 0.0f) {
                    float uvX = shadowComp.cascadeAtlasRects[c].x;
                    float uvY = shadowComp.cascadeAtlasRects[c].y;
                    float uvH = shadowComp.cascadeAtlasRects[c].w;

                    VkImageCopy depthRegion{};
                    depthRegion.srcSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
                    depthRegion.dstSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
                    depthRegion.srcOffset = { static_cast<int32_t>(uvX * SHADOW_ATLAS_SIZE), static_cast<int32_t>(uvY * SHADOW_ATLAS_SIZE), 0 };
                    depthRegion.dstOffset = { static_cast<int32_t>(uvX * SHADOW_ATLAS_SIZE), static_cast<int32_t>(uvY * SHADOW_ATLAS_SIZE), 0 };
                    depthRegion.extent = { static_cast<uint32_t>(uvW * SHADOW_ATLAS_SIZE), static_cast<uint32_t>(uvH * SHADOW_ATLAS_SIZE), 1 };
                    depthRegions.push_back(depthRegion);

                    VkImageCopy colorRegion = depthRegion;
                    colorRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    colorRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    colorRegions.push_back(colorRegion);
                }
            }
        }

        if (depthRegions.empty()) return;

        uint32_t fIdx = context.frameIndex;
        auto srcDepth = drawData->DirectionLightShadow.staticShadowAtlas[fIdx].get();
        auto dstDepth = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();
        auto srcColor = drawData->DirectionLightShadow.staticShadowColorAtlas[fIdx].get();
        auto dstColor = drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get();

        srcDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, false);
        dstDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, true);

        srcColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, false);
        dstColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, true);

        vkCmdCopyImage(context.cmd, srcDepth->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstDepth->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(depthRegions.size()), depthRegions.data());
        vkCmdCopyImage(context.cmd, srcColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(colorRegions.size()), colorRegions.data());

        dstDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, false);
        dstColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, false);
    }
}