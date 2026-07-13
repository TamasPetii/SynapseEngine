#pragma once
#include <string>
#include <cstdint>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    struct TextureNodeState {
        bool isVisible = false;
        std::string name = "";

        uint32_t width = 0;
        uint32_t height = 0;

        uint32_t engineTextureId = 0xFFFFFFFF;
        TextureHandle textureHandle = InvalidTextureHandle;
    };

    struct TextureGraphState {
        TextureNodeState previewNode;
    };
}