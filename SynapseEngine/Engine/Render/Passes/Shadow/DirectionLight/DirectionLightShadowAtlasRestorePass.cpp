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

namespace Syn 
{
    bool DirectionLightShadowAtlasRestorePass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    void DirectionLightShadowAtlasRestorePass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto srcDepth = drawData->DirectionLightShadow.staticShadowAtlas[fIdx].get();
        auto dstDepth = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();
        auto srcColor = drawData->DirectionLightShadow.staticShadowColorAtlas[fIdx].get();
        auto dstColor = drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get();

        srcDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, false);
        dstDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, true);

        VkImageCopy depthCopyRegion{};
        depthCopyRegion.srcSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
        depthCopyRegion.dstSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
        depthCopyRegion.extent = { SHADOW_ATLAS_SIZE, SHADOW_ATLAS_SIZE, 1 };
        vkCmdCopyImage(context.cmd, srcDepth->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstDepth->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &depthCopyRegion);

        dstDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, false);
        srcColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, false);
        dstColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, true);

        VkImageCopy colorCopyRegion{};
        colorCopyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        colorCopyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        colorCopyRegion.extent = { SHADOW_ATLAS_SIZE, SHADOW_ATLAS_SIZE, 1 };
        vkCmdCopyImage(context.cmd, srcColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &colorCopyRegion);

        dstColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, false);
    }
}