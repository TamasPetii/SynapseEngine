#include "ImageUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {

    void ImageUtils::InferSrcState(VkImageLayout layout, VkPipelineStageFlags2& outStage, VkAccessFlags2& outAccess) {
        switch (layout) 
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            outStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            outAccess = 0;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            outStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            outAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            outStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            outAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            outStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            outAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            outStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
            outAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            outStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            outAccess = VK_ACCESS_2_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            outStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            outAccess = 0;
            break;
        default:
            outStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            outAccess = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
            break;
        }
    }

    void ImageUtils::InsertBarrier(VkCommandBuffer cmd, const ImageBarrierInfo& info) {
        VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        barrier.srcStageMask = info.srcStage;
        barrier.srcAccessMask = info.srcAccess;
        barrier.dstStageMask = info.dstStage;
        barrier.dstAccessMask = info.dstAccess;
        barrier.oldLayout = info.oldLayout;
        barrier.newLayout = info.newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = info.image;
        barrier.subresourceRange.aspectMask = info.aspectMask;
        barrier.subresourceRange.baseMipLevel = info.baseMipLevel;
        barrier.subresourceRange.levelCount = info.levelCount;
        barrier.subresourceRange.baseArrayLayer = info.baseArrayLayer;
        barrier.subresourceRange.layerCount = info.layerCount;

        VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    void ImageUtils::TransitionLayout(VkCommandBuffer cmd, const LayoutTransitionInfo& info) {
        InsertBarrier(cmd, info);
    }

    void ImageUtils::CopyImage(VkCommandBuffer cmd, const ImageCopyInfo& info) {
        VkImageBlit2 blit{ VK_STRUCTURE_TYPE_IMAGE_BLIT_2 };
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { (int32_t)info.srcSize.width, (int32_t)info.srcSize.height, (int32_t)info.srcSize.depth };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = info.srcMipLevel;
        blit.srcSubresource.baseArrayLayer = info.srcBaseLayer;
        blit.srcSubresource.layerCount = info.layerCount;

        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { (int32_t)info.dstSize.width, (int32_t)info.dstSize.height, (int32_t)info.dstSize.depth };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = info.dstMipLevel;
        blit.dstSubresource.baseArrayLayer = info.dstBaseLayer;
        blit.dstSubresource.layerCount = info.layerCount;

        VkBlitImageInfo2 blitInfo{ VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2 };
        blitInfo.srcImage = info.srcImage;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.dstImage = info.dstImage;
        blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blitInfo.filter = info.filter;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blit;

        vkCmdBlitImage2(cmd, &blitInfo);
    }

    void ImageUtils::CopyImageToBuffer(VkCommandBuffer cmd, const ImageToBufferCopyInfo& info) {
        VkBufferImageCopy2 region{ VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2 };
        region.bufferOffset = info.bufferOffset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = info.aspectMask;
        region.imageSubresource.mipLevel = info.srcMipLevel;
        region.imageSubresource.baseArrayLayer = info.srcBaseLayer;
        region.imageSubresource.layerCount = info.layerCount;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { info.extent.width, info.extent.height, info.extent.depth };

        VkCopyImageToBufferInfo2 copyInfo{ VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2 };
        copyInfo.srcImage = info.srcImage;
        copyInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        copyInfo.dstBuffer = info.dstBuffer;
        copyInfo.regionCount = 1;
        copyInfo.pRegions = &region;

        vkCmdCopyImageToBuffer2(cmd, &copyInfo);
    }

    void ImageUtils::GenerateMipMaps(VkCommandBuffer cmd, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels) {
        auto physicalDevice = ServiceLocator::GetVkContext()->GetPhysicalDevice();
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice->Handle(), format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            return;
        }

        int32_t mipWidth = width;
        int32_t mipHeight = height;

        for (uint32_t i = 1; i < mipLevels; i++) {
            ImageBarrierInfo barrier{};
            barrier.image = image;
            barrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.baseMipLevel = i - 1;
            barrier.levelCount = 1;
            barrier.baseArrayLayer = 0;
            barrier.layerCount = 1;

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT;

            InsertBarrier(cmd, barrier);

            ImageCopyInfo copyInfo{};
            copyInfo.srcImage = image;
            copyInfo.dstImage = image;

            copyInfo.srcSize = { 
                (uint32_t)mipWidth, 
                (uint32_t)mipHeight, 1 };

            copyInfo.dstSize = {
                (uint32_t)(mipWidth > 1 ? mipWidth / 2 : 1),
                (uint32_t)(mipHeight > 1 ? mipHeight / 2 : 1),
                1
            };

            copyInfo.srcMipLevel = i - 1;
            copyInfo.dstMipLevel = i;
            copyInfo.srcBaseLayer = 0;
            copyInfo.dstBaseLayer = 0;
            copyInfo.layerCount = 1;

            CopyImage(cmd, copyInfo);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.srcAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            barrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;

            InsertBarrier(cmd, barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        ImageBarrierInfo finalBarrier{};
        finalBarrier.image = image;
        finalBarrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        finalBarrier.baseMipLevel = mipLevels - 1;
        finalBarrier.levelCount = 1;
        finalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        finalBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        finalBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        finalBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        finalBarrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;

        InsertBarrier(cmd, finalBarrier);
    }

    uint32_t ImageUtils::CalculateMipLevels(uint32_t width, uint32_t height) {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    }
}