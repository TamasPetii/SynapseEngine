#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    struct SYN_API ImageViewConfig {
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = VK_REMAINING_MIP_LEVELS;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS;
        VkComponentMapping swizzle = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        bool perMipViews = false;
    };

    struct SYN_API ImageConfig {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;

        VkImageType type = VK_IMAGE_TYPE_2D;
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags usage = 0;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        bool generateMipMaps = false;

        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        VmaAllocationCreateFlags allocationFlags = 0;
        VkImageCreateFlags flags = 0;

        std::unordered_map<std::string, ImageViewConfig> imageViewConfigs;
        void AddView(const std::string& name, const ImageViewConfig& config) {
            imageViewConfigs[name] = config;
        }
    };
}