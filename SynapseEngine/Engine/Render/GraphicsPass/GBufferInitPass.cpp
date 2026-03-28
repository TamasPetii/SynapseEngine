#include "GBufferInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    void GBufferInitPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        if (!group) return;

        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        struct TargetClearInfo {
            std::string name;
            VkClearValue clearValue;
        };

        std::vector<TargetClearInfo> colorTargets = {
            { RenderTargetNames::Main,             VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::TransparentAccum, VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}} },
            { RenderTargetNames::TransparentReveal,VkClearValue{.color = {.float32 = {1.0f, 1.0f, 1.0f, 1.0f}}} },
            { RenderTargetNames::EntityIndex,      VkClearValue{.color = {.uint32 = {0xFFFFFFFF, 0, 0, 0}}} }
        };

        for (const auto& target : colorTargets)
        {
            if (auto img = group->GetImage(target.name))
            {
                _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                        .imageView = img->GetView(Vk::ImageViewNames::Default),
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .clearValue = target.clearValue,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    }));

                _imageTransitions.push_back({
                    .image = img,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .discardContent = true
                    });
            }
        }

        if (auto depthImg = group->GetImage(RenderTargetNames::Depth)) 
        {
            _depthAttachment = Vk::RenderUtils::CreateAttachment({
                .imageView = depthImg->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .clearValue = VkClearValue{.depthStencil = {1.0f, 0}},
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                });

            _imageTransitions.push_back({
                .image = depthImg,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .discardContent = true
                });
        }

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = _depthAttachment.has_value() ? &_depthAttachment.value() : nullptr,
            .layerCount = 1
        };
    }
}