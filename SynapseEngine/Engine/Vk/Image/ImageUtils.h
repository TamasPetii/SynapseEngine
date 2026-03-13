#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {

    struct SYN_API ImageBarrierInfo {
        VkImage image = VK_NULL_HANDLE;

        VkPipelineStageFlags2 srcStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 srcAccess = VK_ACCESS_2_NONE;
        VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkPipelineStageFlags2 dstStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 dstAccess = VK_ACCESS_2_NONE;
        VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t baseMipLevel = 0;
        uint32_t levelCount = VK_REMAINING_MIP_LEVELS;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS;
    };

	using LayoutTransitionInfo = ImageBarrierInfo;

    struct SYN_API ImageCopyInfo {
        VkImage srcImage = VK_NULL_HANDLE;
        VkImage dstImage = VK_NULL_HANDLE;

        VkExtent3D srcSize = { 0, 0, 1 };
        VkExtent3D dstSize = { 0, 0, 1 };

        uint32_t srcMipLevel = 0;
        uint32_t dstMipLevel = 0;
        uint32_t srcBaseLayer = 0;
        uint32_t dstBaseLayer = 0;
        uint32_t layerCount = 1;

        VkFilter filter = VK_FILTER_NEAREST;
    };

    struct SYN_API ImageToBufferCopyInfo {
        VkImage srcImage = VK_NULL_HANDLE;
        VkBuffer dstBuffer = VK_NULL_HANDLE;

        VkExtent3D extent = { 0, 0, 1 };

        uint32_t srcMipLevel = 0;
        uint32_t srcBaseLayer = 0;
        uint32_t layerCount = 1;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        VkDeviceSize bufferOffset = 0;
    };

    class SYN_API ImageUtils {
    public:
        static void InferSrcState(VkImageLayout layout, VkPipelineStageFlags2& outStage, VkAccessFlags2& outAccess);
        static void InsertBarrier(VkCommandBuffer cmd, const ImageBarrierInfo& info);
        static void TransitionLayout(VkCommandBuffer cmd, const LayoutTransitionInfo& info);
        static void CopyImage(VkCommandBuffer cmd, const ImageCopyInfo& info);
        static void CopyImageToBuffer(VkCommandBuffer cmd, const ImageToBufferCopyInfo& info);
        static void GenerateMipMaps(VkCommandBuffer cmd, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels);
        static uint32_t CalculateMipLevels(uint32_t width, uint32_t height);
    };
}