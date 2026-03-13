#include "CopyToSwapchainPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn {

    void CopyToSwapchainPass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto mainImage = group->GetImage(RenderTargetNames::Main);
        auto swapchainImage = swapChain->GetImage(context.swapchainImageIndex);

        _imageTransitions.push_back({
            .image = mainImage,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT
            });

        _imageTransitions.push_back({
            .image = swapchainImage,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .discardContent = true
            });


    }

    void CopyToSwapchainPass::Transfer(const RenderContext& context) {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto mainImage = group->GetImage(RenderTargetNames::Main);
        auto swapchainImage = swapChain->GetImage(context.swapchainImageIndex);

        Vk::ImageCopyInfo copyInfo{};
        copyInfo.srcImage = mainImage->Handle();
        copyInfo.dstImage = swapchainImage->Handle();
        copyInfo.srcSize = mainImage->GetExtent();
        copyInfo.dstSize = swapchainImage->GetExtent();
        copyInfo.layerCount = 1;

        Vk::ImageUtils::CopyImage(context.cmd, copyInfo);
    }
}