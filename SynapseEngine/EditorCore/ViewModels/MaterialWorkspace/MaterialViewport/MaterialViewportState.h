// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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