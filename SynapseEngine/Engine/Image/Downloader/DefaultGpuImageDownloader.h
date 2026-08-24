#pragma once
#include "Engine/SynApi.h"
#include "IGpuImageDownloader.h"

namespace Syn
{
    class SYN_API DefaultGpuImageDownloader : public IGpuImageDownloader
    {
    public:
        ImageDownloadResult Download(Vk::Image& image, VkCommandBuffer cmd) override;
    };
}