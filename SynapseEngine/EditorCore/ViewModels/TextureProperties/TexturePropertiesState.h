#pragma once
#include "Engine/Image/Data/Cpu/CpuTextureData.h"
#include <string>

namespace Syn {
    struct TexturePropertiesState {
        bool hasSelection = false;
        std::string textureName = "";

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t format = 0;
        bool isCompressed = false;
    };
}