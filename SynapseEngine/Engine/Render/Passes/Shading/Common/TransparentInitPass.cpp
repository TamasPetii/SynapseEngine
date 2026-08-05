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

        std::vector<TargetClearInfo> targetsToClear = {
            { RenderTargetNames::TransparentAccum, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::TransparentReveal, VkClearValue{.color = {.float32 = {1.0f, 1.0f, 1.0f, 1.0f}}} },
            { RenderTargetNames::TransparentDepth, VkClearValue{.depthStencil = {1.0f, 0}}, true },

            { RenderTargetNames::TransparentAccumMSAA, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::TransparentRevealMSAA, VkClearValue{.color = {.float32 = {1.0f, 1.0f, 1.0f, 1.0f}}} }
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