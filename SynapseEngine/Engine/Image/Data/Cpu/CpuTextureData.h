#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Image/Data/Cooked/CookedImage.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"

namespace Syn
{
    struct SYN_API CpuTextureData
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;
        bool isCompressed = false;
    };
}