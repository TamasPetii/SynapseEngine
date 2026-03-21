#pragma once
#include "Engine/SynApi.h"
#include "IGpuAnimationUploader.h"
#include <memory>

namespace Syn
{
    class SYN_API DefaultGpuAnimationUploader : public IGpuAnimationUploader
    {
    public:
        DefaultGpuAnimationUploader() = default;
        ~DefaultGpuAnimationUploader() override = default;

        AnimationUploadResult Upload(const GpuBatchedAnimation& data, VkCommandBuffer cmd) override;
    };
}