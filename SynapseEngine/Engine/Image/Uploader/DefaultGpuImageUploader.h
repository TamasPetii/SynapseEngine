#pragma once
#include "Engine/SynApi.h"
#include "IGpuImageUploader.h"

namespace Syn
{
    class SYN_API DefaultGpuImageUploader : public IGpuImageUploader
    {
    public:
        DefaultGpuImageUploader() = default;
        ~DefaultGpuImageUploader() override = default;

        ImageUploadResult Upload(const GpuImage& data, VkCommandBuffer cmd) override;
    };
}