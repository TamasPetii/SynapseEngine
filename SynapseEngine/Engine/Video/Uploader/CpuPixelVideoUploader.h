#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Uploader/IGpuVideoUploader.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API CpuPixelVideoUploader : public IGpuVideoUploader
    {
    public:
        CpuPixelVideoUploader(uint32_t width, uint32_t height, uint32_t bufferCount);
        ~CpuPixelVideoUploader() override = default;

        VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd) override;
    private:
        std::vector<std::shared_ptr<Vk::Image>> _textures;
        uint32_t _frameIndex = 0;

        uint32_t _width;
        uint32_t _height;
    };
}