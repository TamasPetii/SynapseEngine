#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include "Data/RenderScene.h"

namespace Syn {
    struct SYN_API RenderContext {
        VkCommandBuffer cmd;
        uint32_t frameIndex;
        uint32_t swapchainImageIndex;
        const RenderScene& scene;
    };
}