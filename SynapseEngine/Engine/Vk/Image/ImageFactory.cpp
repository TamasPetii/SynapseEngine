#include "ImageFactory.h"
#include "ImageUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "ImageViewNames.h"

namespace Syn::Vk {

    void ImageFactory::Allocate(Image* image) {
        auto context = ServiceLocator::GetVkContext();
        auto device = context->GetDevice();
        VmaAllocator allocator = device->GetAllocator();

        if (image->_config.generateMipMaps && image->_config.mipLevels <= 1) {
            image->_config.mipLevels = ImageUtils::CalculateMipLevels(image->_config.width, image->_config.height);
        }

        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = image->_config.type;
        imageInfo.extent.width = image->_config.width;
        imageInfo.extent.height = image->_config.height;
        imageInfo.extent.depth = image->_config.depth;
        imageInfo.mipLevels = image->_config.mipLevels;
        imageInfo.arrayLayers = image->_config.arrayLayers;
        imageInfo.format = image->_config.format;
        imageInfo.tiling = image->_config.tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = image->_config.usage;
        imageInfo.samples = image->_config.samples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = image->_config.flags;

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = image->_config.memoryUsage;
        allocCreateInfo.flags = image->_config.allocationFlags;

        SYN_VK_ASSERT_MSG(vmaCreateImage(allocator, &imageInfo, &allocCreateInfo, &image->_handle, &image->_allocation, nullptr), "Failed to create VMA Image");

        image->_allocator = allocator;

        CreateViews(image);
    }

    void ImageFactory::CreateViews(Image* image) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        if (!image->_config.imageViewConfigs.contains(ImageViewNames::Default)) {
            ImageViewConfig defaultConfig;
            defaultConfig.viewType = (image->_config.arrayLayers > 1) ?
                (image->_config.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT ?
                    VK_IMAGE_VIEW_TYPE_CUBE :
                    VK_IMAGE_VIEW_TYPE_2D_ARRAY) :
                VK_IMAGE_VIEW_TYPE_2D;
            image->_config.imageViewConfigs[ImageViewNames::Default] = defaultConfig;
        }

        auto createView = [&](const std::string& name, const ImageViewConfig& config) {
            VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            viewInfo.image = image->_handle;
            viewInfo.viewType = config.viewType;
            viewInfo.format = image->_config.format;
            viewInfo.subresourceRange.aspectMask = image->_config.aspectMask;
            viewInfo.subresourceRange.baseMipLevel = config.baseMipLevel;
            viewInfo.subresourceRange.levelCount = config.mipLevelCount == VK_REMAINING_MIP_LEVELS ? (image->_config.mipLevels - config.baseMipLevel) : config.mipLevelCount;
            viewInfo.subresourceRange.baseArrayLayer = config.baseArrayLayer;
            viewInfo.subresourceRange.layerCount = config.layerCount == VK_REMAINING_ARRAY_LAYERS ? image->_config.arrayLayers : config.layerCount;
            viewInfo.components = config.swizzle;

            VkImageView viewHandle;
            SYN_VK_ASSERT_MSG(vkCreateImageView(device->Handle(), &viewInfo, nullptr, &viewHandle), ("Failed to create Image View: " + name).c_str());
            image->_imageViews[name] = viewHandle;
            };

        for (const auto& [name, viewConfig] : image->_config.imageViewConfigs) {
            createView(name, viewConfig);

            if (viewConfig.perMipViews) {
                uint32_t actualMipCount = (viewConfig.mipLevelCount == VK_REMAINING_MIP_LEVELS) ?
                    image->_config.mipLevels : viewConfig.mipLevelCount;

                for (uint32_t i = 0; i < actualMipCount; i++) {
                    uint32_t currentMip = viewConfig.baseMipLevel + i;
                    if (currentMip >= image->_config.mipLevels) break;

                    ImageViewConfig mipConfig = viewConfig;
                    mipConfig.baseMipLevel = currentMip;
                    mipConfig.mipLevelCount = 1;

                    std::string mipName = name + ImageViewNames::Mip + std::to_string(currentMip);
                    createView(mipName, mipConfig);
                }
            }
        }
    }

    std::unique_ptr<Image> ImageFactory::Create(const ImageConfig& config) {
        return std::make_unique<Image>(config);
    }

    std::unique_ptr<Image> ImageFactory::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, bool mipmaps) {
        ImageConfig config;
        config.width = width;
        config.height = height;
        config.format = format;
        config.usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        config.generateMipMaps = mipmaps;
        config.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        return Create(config);
    }

    std::unique_ptr<Image> ImageFactory::CreateAttachment(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) {
        ImageConfig config;
        config.width = width;
        config.height = height;
        config.format = format;
        config.usage = usage;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        config.mipLevels = 1;

        if (format == VK_FORMAT_D32_SFLOAT || 
            format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
            format == VK_FORMAT_D24_UNORM_S8_UINT || 
            format == VK_FORMAT_D16_UNORM) {
            config.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else {
            config.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        return Create(config);
    }

    std::unique_ptr<Image> ImageFactory::CreateCubemap(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) {
        ImageConfig config;
        config.width = width;
        config.height = height;
        config.format = format;
        config.usage = usage;
        config.arrayLayers = 6;
        config.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        return Create(config);
    }
}