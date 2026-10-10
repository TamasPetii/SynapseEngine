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

#include "HizInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn {

    void HizInitPass::PrepareFrame(const RenderContext& context) {
        //Using prevous frame's depth pyramid!
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
        auto drawData = context.scene->GetSceneDrawData();

        {
            auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
            if (depthPyramid && depthPyramid->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED) 
            {
                depthPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_CLEAR_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    true
                );

                Vk::ImageClearColorInfo clearInfo{};
                clearInfo.image = depthPyramid->Handle();
                clearInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                clearInfo.clearColor = { {1.0f, 1.0f, 0.0f, 0.0f} };
                clearInfo.levelCount = depthPyramid->GetConfig().generateMipMaps ? Vk::ImageUtils::CalculateMipLevels(depthPyramid->GetConfig().width, depthPyramid->GetConfig().height) : 1;

                Vk::ImageUtils::ClearColorImage(context.cmd, clearInfo);

                depthPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT,
                    false
                );
            }  
        }

        {
            auto& shadowPyramid = drawData->DirectionLightShadow.shadowDepthPyramid[context.frameIndex];
            if (shadowPyramid && shadowPyramid->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                shadowPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_CLEAR_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    true
                );

                Vk::ImageClearColorInfo clearInfo{};
                clearInfo.image = shadowPyramid->Handle();
                clearInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                clearInfo.clearColor = { {1.0f, 1.0f, 0.0f, 0.0f} };
                clearInfo.levelCount = shadowPyramid->GetConfig().mipLevels;

                Vk::ImageUtils::ClearColorImage(context.cmd, clearInfo);

                shadowPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT,
                    false
                );
            }
        }

        {
            auto& shadowPyramid = drawData->SpotLightShadow.shadowDepthPyramid[context.frameIndex];
            if (shadowPyramid && shadowPyramid->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                shadowPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_CLEAR_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    true
                );

                Vk::ImageClearColorInfo clearInfo{};
                clearInfo.image = shadowPyramid->Handle();
                clearInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                clearInfo.clearColor = { {1.0f, 1.0f, 0.0f, 0.0f} };
                clearInfo.levelCount = shadowPyramid->GetConfig().mipLevels;

                Vk::ImageUtils::ClearColorImage(context.cmd, clearInfo);

                shadowPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT,
                    false
                );
            }
        }

        {
            auto& shadowPyramid = drawData->PointLightShadow.shadowDepthPyramid[context.frameIndex];
            if (shadowPyramid && shadowPyramid->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                shadowPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_CLEAR_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    true
                );

                Vk::ImageClearColorInfo clearInfo{};
                clearInfo.image = shadowPyramid->Handle();
                clearInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                clearInfo.clearColor = { {1.0f, 1.0f, 0.0f, 0.0f} };
                clearInfo.levelCount = shadowPyramid->GetConfig().mipLevels;

                Vk::ImageUtils::ClearColorImage(context.cmd, clearInfo);

                shadowPyramid->TransitionLayout(
                    context.cmd,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT,
                    false
                );
            }
        }     
    }
}