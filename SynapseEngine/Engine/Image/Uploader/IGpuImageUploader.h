#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"

namespace Syn
{
    struct SYN_API Texture {
        std::shared_ptr<Vk::Image> image;
    };

    struct SYN_API ImageUploadResult {
        std::shared_ptr<Texture> texture;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
        bool requiresGraphicsQueue = false;
    };

    class SYN_API IGpuImageUploader
    {
    public:
        virtual ~IGpuImageUploader() = default;
        virtual ImageUploadResult Upload(const GpuImage& data, VkCommandBuffer cmd) = 0;
    };
}