#pragma once
#include "Engine/SynApi.h"
#include "IGpuModelUploader.h"
#include <memory>

namespace Syn
{
    class SYN_API DefaultGpuModelUploader : public IGpuModelUploader
    {
    public:
        DefaultGpuModelUploader() = default;
        ~DefaultGpuModelUploader() override = default;

        ModelUploadResult Upload(const GpuBatchedModel& data, VkCommandBuffer cmd) override;
    };
}