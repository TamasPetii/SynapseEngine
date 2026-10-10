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

#include "DirectionLightShadowAtlasClearPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn
{
    bool DirectionLightShadowAtlasClearPass::ShouldExecute(const RenderContext& context) const {
        auto drawData = context.scene->GetSceneDrawData();
        return drawData && drawData->DirectionLightShadow.visibleLightCount > 0;
    }

    void DirectionLightShadowAtlasClearPass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto dstDepth = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();
        auto dstColor = drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get();

        dstDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, false);
        dstColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, false);

        VkClearDepthStencilValue depthClearValue = { 1.0f, 0 };
        VkClearColorValue colorClearValue = { {1.0f, 1.0f, 1.0f, 1.0f} };

        VkImageSubresourceRange depthRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        VkImageSubresourceRange colorRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        vkCmdClearDepthStencilImage(context.cmd, dstDepth->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &depthClearValue, 1, &depthRange);
        vkCmdClearColorImage(context.cmd, dstColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &colorClearValue, 1, &colorRange);

        dstDepth->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, false);
        dstColor->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, false);
    }
}