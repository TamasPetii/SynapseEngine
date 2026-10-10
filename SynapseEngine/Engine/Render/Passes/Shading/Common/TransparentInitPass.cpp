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

#include "TransparentInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"
#include <map>
#include <set>

namespace Syn {

    void TransparentInitPass::Execute(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        if (!group) return;

        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };

        struct TargetClearInfo {
            std::string name;
            VkClearValue clearValue;
            bool isDepth = false;
        };

        std::vector<TargetClearInfo> baseTargets = {
            { RenderTargetNames::TransparentAccum, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::TransparentReveal, VkClearValue{.color = {.float32 = {1.0f, 1.0f, 1.0f, 1.0f}}} },
            { RenderTargetNames::TransparentDepth, VkClearValue{.depthStencil = {1.0f, 0}}, true }
        };

        std::vector<TargetClearInfo> msaaTargets = {
            { RenderTargetNames::TransparentAccumMSAA, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::TransparentRevealMSAA, VkClearValue{.color = {.float32 = {1.0f, 1.0f, 1.0f, 1.0f}}} }
        };

        auto processTargets = [&](const std::vector<TargetClearInfo>& targets) {
            std::vector<VkRenderingAttachmentInfo> colorAttachments;
            VkRenderingAttachmentInfo depthAttachment{};
            bool hasDepth = false;

            for (const auto& target : targets) {
                if (auto img = group->GetImage(target.name)) {
                    VkImageLayout newLayout = target.isDepth ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    VkPipelineStageFlags2 dstStage = target.isDepth ? (VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT) : VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                    VkAccessFlags2 dstAccess = target.isDepth ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

                    img->TransitionLayout(context.cmd, newLayout, dstStage, dstAccess, true);

                    VkRenderingAttachmentInfo attach = Vk::RenderUtils::CreateAttachment({
                        .imageView = img->GetView(Vk::ImageViewNames::Default),
                        .layout = newLayout,
                        .clearValue = target.clearValue,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                        });

                    if (target.isDepth) {
                        depthAttachment = attach;
                        hasDepth = true;
                    }
                    else {
                        colorAttachments.push_back(attach);
                    }
                }
            }

            if (!colorAttachments.empty() || hasDepth) {
                Vk::RenderingInfoConfig renderInfo{
                    .renderArea = extent,
                    .colorAttachments = colorAttachments,
                    .depthAttachment = hasDepth ? &depthAttachment : nullptr,
                    .layerCount = 1
                };

                Vk::RenderUtils::BeginRendering(context.cmd, renderInfo);
                Vk::RenderUtils::EndRendering(context.cmd);
            }
        };

        processTargets(baseTargets);
        processTargets(msaaTargets);
    }
}