#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>

namespace Syn
{
    struct ImageDownloadResult {
        std::shared_ptr<Vk::Buffer> stagingBuffer;
        size_t sizeInBytes = 0;
    };

    class SYN_API IGpuImageDownloader
    {
    public:
        virtual ~IGpuImageDownloader() = default;
        virtual ImageDownloadResult Download(Vk::Image& image, VkCommandBuffer cmd) = 0;
    };
}