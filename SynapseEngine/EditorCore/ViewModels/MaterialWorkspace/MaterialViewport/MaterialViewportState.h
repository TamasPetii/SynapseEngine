#pragma once
#include <string>
#include <cstdint>
#include "EditorCore/Types/TextureHandle.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    struct MaterialViewportState {
        uint32_t width = 0;
        uint32_t height = 0;

        TextureHandle textureId = InvalidTextureHandle;

        std::string currentGroup = RenderTargetGroupNames::Main;
        std::string currentTarget = RenderTargetNames::Main;
        std::string currentView = Vk::ImageViewNames::Default;

        bool isHovered = false;
        bool isFocused = false;
    };
}