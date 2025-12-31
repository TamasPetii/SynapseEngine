#pragma once
#include "Engine/SynApi.h"
#include <volk.h>

namespace Syn::Vk {

    struct SYN_API BufferCopyInfo {
        VkBuffer srcBuffer = VK_NULL_HANDLE;
        VkBuffer dstBuffer = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize srcOffset = 0;
        VkDeviceSize dstOffset = 0;
    };

    struct SYN_API BufferToImageCopyInfo {
        VkBuffer srcBuffer = VK_NULL_HANDLE;
        VkImage dstImage = VK_NULL_HANDLE;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;

        VkDeviceSize bufferOffset = 0;

        uint32_t mipLevel = 0;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    };

    struct SYN_API BufferBarrierInfo {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize size = VK_WHOLE_SIZE;
        VkDeviceSize offset = 0;

        VkPipelineStageFlags2 srcStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 srcAccess = VK_ACCESS_2_NONE;
        VkPipelineStageFlags2 dstStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 dstAccess = VK_ACCESS_2_NONE;

        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    class SYN_API BufferUtils {
    public:
        static void CopyBuffer(VkCommandBuffer cmd, const BufferCopyInfo& info);
        static void CopyBufferToImage(VkCommandBuffer cmd, const BufferToImageCopyInfo& info);
        static void InsertBarrier(VkCommandBuffer cmd, const BufferBarrierInfo& info);
    };
}