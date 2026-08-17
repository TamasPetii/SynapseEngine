#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Texture.h"
#include <vulkan/vulkan.h>

namespace Syn 
{
    struct SYN_API EnvironmentBakeContext {
        VkCommandBuffer cmd;
        Texture* sourceEquirectangular;
        Texture* baseCubemap;
        Texture* irradianceMap;
        Texture* prefilteredMap;
    };
}