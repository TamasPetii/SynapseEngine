#pragma once
#include "Image.h"

namespace Syn::Vk {

    class SYN_API ImageFactory {
    public:
        static void Allocate(Image* image);
        static std::unique_ptr<Image> Create(const ImageConfig& config);
        static std::unique_ptr<Image> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, bool mipmaps = true);
        static std::unique_ptr<Image> CreateAttachment(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
        static std::unique_ptr<Image> CreateCubemap(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
    };
}