#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include "Engine/Scene/Scene.h"
#include "RenderTargetManager.h"

namespace Syn {
    struct SYN_API RenderContext {
        VkCommandBuffer cmd;
        uint32_t frameIndex;
        uint32_t framesInFlight;
        uint32_t swapchainImageIndex;
        Scene* scene;
        RenderTargetManager* renderTargetManager;
    };
}