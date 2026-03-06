#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Gpu/GpuModelBuffers.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include <memory>

namespace Syn
{
    struct SYN_API ModelUploadResult {
        ModelUploadResult() = default;
        GpuModelBuffers hardwareBuffers;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
    };

    class SYN_API IGpuModelUploader
    {
    public:
        virtual ~IGpuModelUploader() = default;
        virtual ModelUploadResult Upload(const GpuBatchedModel& data, VkCommandBuffer cmd) = 0;
    };
}