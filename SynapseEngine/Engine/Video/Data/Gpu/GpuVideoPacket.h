#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace Syn
{
    struct SYN_API GpuVideoPacket {
        std::vector<uint8_t> bitstreamData;
        int64_t pts = 0;
        int64_t dts = 0;
        bool isKeyFrame = false;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };
}