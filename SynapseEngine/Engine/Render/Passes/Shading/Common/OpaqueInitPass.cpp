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

#include "OpaqueInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include <map>
#include <set>

namespace Syn {

    void OpaqueInitPass::Execute(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        if (!group) return;

        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };

        struct TargetClearInfo {
            std::string name;
            VkClearValue clearValue;
            bool isDepth = false;
        };

        std::vector<TargetClearInfo> targetsToClear = {
            { RenderTargetNames::Main, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}} },
            { RenderTargetNames::ColorMetallic, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::NormalRoughness, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::EmissiveAo, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::EntityIndex, VkClearValue{.color = {.uint32 = {0xFFFFFFFF, 0, 0, 0}}} },
            { RenderTargetNames::OpaqueDepth, VkClearValue{.depthStencil = {1.0f, 0}}, true },

            { RenderTargetNames::MainMSAA, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}} },
            { RenderTargetNames::EntityIndexMSAA, VkClearValue{.color = {.uint32 = {0xFFFFFFFF, 0, 0, 0}}} },
            { RenderTargetNames::OpaqueDepthMSAA, VkClearValue{.depthStencil = {1.0f, 0}}, true }
        };

        std::map<VkSampleCountFlagBits, std::vector<VkRenderingAttachmentInfo>> colorAttachmentsMap;
        std::map<VkSampleCountFlagBits, VkRenderingAttachmentInfo> depthAttachmentsMap;

        for (const auto& target : targetsToClear) {
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

                VkSampleCountFlagBits samples = img->GetConfig().samples;
                if (target.isDepth) {
                    depthAttachmentsMap[samples] = attach;
                }
                else {
                    colorAttachmentsMap[samples].push_back(attach);
                }
            }
        }

        std::set<VkSampleCountFlagBits> allSamples;
        for (const auto& pair : colorAttachmentsMap) allSamples.insert(pair.first);
        for (const auto& pair : depthAttachmentsMap) allSamples.insert(pair.first);

        for (VkSampleCountFlagBits samples : allSamples) {
            Vk::RenderingInfoConfig renderInfo{
                .renderArea = extent,
                .colorAttachments = colorAttachmentsMap[samples],
                .depthAttachment = depthAttachmentsMap.contains(samples) ? &depthAttachmentsMap[samples] : nullptr,
                .layerCount = 1
            };

            Vk::RenderUtils::BeginRendering(context.cmd, renderInfo);
            Vk::RenderUtils::EndRendering(context.cmd);
        }
    }
}