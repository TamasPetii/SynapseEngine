#include "BufferUtils.h"

namespace Syn::Vk {

    void BufferUtils::CopyBuffer(VkCommandBuffer cmd, const BufferCopyInfo& info) {
        VkBufferCopy region{};
        region.srcOffset = info.srcOffset;
        region.dstOffset = info.dstOffset;
        region.size = info.size;

        vkCmdCopyBuffer(cmd, info.srcBuffer, info.dstBuffer, 1, &region);
    }

    void BufferUtils::CopyBufferToImage(VkCommandBuffer cmd, const BufferToImageCopyInfo& info) {
        VkBufferImageCopy2 region{ VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2 };

        region.bufferOffset = info.bufferOffset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = info.aspectMask;
        region.imageSubresource.mipLevel = info.mipLevel;
        region.imageSubresource.baseArrayLayer = info.baseArrayLayer;
        region.imageSubresource.layerCount = info.layerCount;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { info.width, info.height, info.depth };

        VkCopyBufferToImageInfo2 copyInfo{ VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2 };
        copyInfo.srcBuffer = info.srcBuffer;
        copyInfo.dstImage = info.dstImage;
        copyInfo.dstImageLayout = info.dstImageLayout;
        copyInfo.regionCount = 1;
        copyInfo.pRegions = &region;

        vkCmdCopyBufferToImage2(cmd, &copyInfo);
    }

    void BufferUtils::InsertBarrier(VkCommandBuffer cmd, const BufferBarrierInfo& info) {
        VkBufferMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };

        barrier.srcStageMask = info.srcStage;
        barrier.srcAccessMask = info.srcAccess;
        barrier.dstStageMask = info.dstStage;
        barrier.dstAccessMask = info.dstAccess;

        barrier.srcQueueFamilyIndex = info.srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = info.dstQueueFamilyIndex;

        barrier.buffer = info.buffer;
        barrier.offset = info.offset;
        barrier.size = info.size;

        VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.bufferMemoryBarrierCount = 1;
        depInfo.pBufferMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }
}