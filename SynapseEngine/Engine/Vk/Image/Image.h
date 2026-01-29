#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {

    class ImageFactory;

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

        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags allocationFlags = 0;
        VkImageCreateFlags flags = 0;

        std::unordered_map<std::string, ImageViewConfig> imageViewConfigs;
        void AddView(const std::string& name, const ImageViewConfig& config) {
            imageViewConfigs[name] = config;
        }
    };

    class SYN_API Image {
    public:
        explicit Image(const ImageConfig& config);
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image&&) = delete;

        VkImage Handle() const { return _handle; }
        VkFormat GetFormat() const { return _config.format; }
        VkExtent3D GetExtent() const { return { _config.width, _config.height, _config.depth }; }
        const ImageConfig& GetConfig() const { return _config; }
        VkImageView GetView(const std::string& name = "") const;

        void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess, bool discardContent = false);
    private:
        ImageConfig _config;

        VkImage _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;

        VkImageLayout _currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 _currentStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 _currentAccess = VK_ACCESS_2_NONE;

        std::unordered_map<std::string, VkImageView> _imageViews;

        friend class ImageFactory;
    };
}