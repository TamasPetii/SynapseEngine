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

#include "TransparentForwardTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    bool TransparentForwardTransitionPass::ShouldExecute(const RenderContext& context) const
    {
        return !context.scene->GetSettings()->debug.enableDebugVisibility;
    }

    void TransparentForwardTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        std::vector<std::string> colorTargets = {
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal,
            RenderTargetNames::TransparentAccumMSAA,
            RenderTargetNames::TransparentRevealMSAA
        };

        for (const auto& name : colorTargets)
        {
            if (auto img = group->GetImage(name)) {
                if (img->GetLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
					_imageTransitions.push_back({
						.image = img,
						.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
						.dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
						.discardContent = false
						});
                }
            }
        }

        std::vector<std::string> depthTargets = {
            RenderTargetNames::OpaqueDepth,
            RenderTargetNames::OpaqueDepthMSAA
        };

        for (const auto& name : depthTargets)
        {
            if (auto depthImg = group->GetImage(name)) {
                if (depthImg->GetLayout() != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
                    _imageTransitions.push_back({
                        .image = depthImg,
                        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                        .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                        .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                        .discardContent = false
                        });
                }
            }
        }
    }
}