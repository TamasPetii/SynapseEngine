#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Gpu/GpuAnimationBuffers.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include <memory>

namespace Syn
{
    struct SYN_API AnimationUploadResult {
        AnimationUploadResult() = default;
        GpuAnimationBuffers hardwareBuffers;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
    };

    class SYN_API IGpuAnimationUploader
    {
    public:
        virtual ~IGpuAnimationUploader() = default;
        virtual AnimationUploadResult Upload(const GpuBatchedAnimation& data, VkCommandBuffer cmd) = 0;
    };
}