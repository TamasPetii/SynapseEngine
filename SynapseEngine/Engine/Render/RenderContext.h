#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include "Engine/Scene/Scene.h"

namespace Syn {
    struct SYN_API RenderContext {
        VkCommandBuffer cmd;
        uint32_t frameIndex;
        uint32_t swapchainImageIndex;
        std::shared_ptr<Scene> scene;
    };
}