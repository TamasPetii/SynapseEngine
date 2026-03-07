#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>
#include "Engine/Image/Data/Common/MipLevelInfo.h"

namespace Syn
{
    struct SYN_API RawImage {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;

        VkFormat format = VK_FORMAT_UNDEFINED;

        bool isCompressed = false;

        std::vector<uint8_t> pixels;
        std::vector<MipLevelInfo> mipData;
    };
}