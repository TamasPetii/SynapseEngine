#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Data/Gpu/GpuVideoPacket.h"
#include "VideoUploadResult.h"
#include <vulkan/vulkan.h>

namespace Syn
{
    class SYN_API IGpuVideoUploader
    {
    public:
        virtual ~IGpuVideoUploader() = default;
        virtual VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd) = 0;
    protected:
        static constexpr uint32_t BUFFER_COUNT = 3;
    };
}