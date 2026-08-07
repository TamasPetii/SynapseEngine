#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Uploader/IGpuVideoUploader.h"

namespace Syn
{
    class SYN_API CpuPixelVideoUploader : public IGpuVideoUploader
    {
    public:
        CpuPixelVideoUploader(uint32_t width, uint32_t height);
        ~CpuPixelVideoUploader() override = default;

        VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd) override;
    private:
        uint32_t _width;
        uint32_t _height;
    };
}