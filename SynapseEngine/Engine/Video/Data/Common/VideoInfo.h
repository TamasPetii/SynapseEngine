#pragma once
#include "Engine/SynApi.h"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace Syn
{
    struct SYN_API VideoInfo {
        uint32_t width = 0;
        uint32_t height = 0;
        double frameRate = 0.0;
        int64_t duration = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };
}