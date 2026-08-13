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

#include "DepthCopyPass.h"
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn 
{
    void DepthCopyPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        if (!group) return;

        auto srcDepth = group->GetImage(RenderTargetNames::OpaqueDepth);
        auto dstDepth = group->GetImage(RenderTargetNames::TransparentDepth);
        if (!srcDepth || !dstDepth) return;

        _imageTransitions.push_back({
            .image = srcDepth,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = dstDepth,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .discardContent = true
            });
    }

    void DepthCopyPass::Transfer(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        if (!group) return;

        auto srcDepth = group->GetImage(RenderTargetNames::OpaqueDepth);
        auto dstDepth = group->GetImage(RenderTargetNames::TransparentDepth);
        if (!srcDepth || !dstDepth) return;

        Vk::ImageCopyInfo copyInfo{};
        copyInfo.srcImage = srcDepth->Handle();
        copyInfo.dstImage = dstDepth->Handle();
        copyInfo.srcSize = { group->GetWidth(), group->GetHeight(), 1 };
        copyInfo.dstSize = { group->GetWidth(), group->GetHeight(), 1 };
        copyInfo.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        Vk::ImageUtils::CopyImage(context.cmd, copyInfo);

        srcDepth->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            false
        );

        dstDepth->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            false
        );
    }
}