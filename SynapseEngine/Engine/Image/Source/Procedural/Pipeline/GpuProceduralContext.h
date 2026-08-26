#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include "Engine/Vk/Image/Image.h"

namespace Syn
{
    struct SYN_API GpuProceduralContext {
        VkCommandBuffer cmd;
        Vk::Image* targetImage;
    };
}