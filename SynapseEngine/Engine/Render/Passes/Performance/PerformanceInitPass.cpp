#include "PerformanceInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {
    void PerformanceInitPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        if (!group) return;

        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        if (auto mainImg = group->GetImage(RenderTargetNames::Main)) {
            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                .imageView = mainImg->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .clearValue = VkClearValue{.color = {.float32 = {0.0f, 0.0f, 0.0f, 0.0f}}},
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                }));

            _imageTransitions.push_back({
                .image = mainImg,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .discardContent = true
                });
        }

        if (auto depthImg = group->GetImage(RenderTargetNames::Depth)) {
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