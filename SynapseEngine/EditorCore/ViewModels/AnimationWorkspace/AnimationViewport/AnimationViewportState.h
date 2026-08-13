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
#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include "EditorCore/Types/TextureHandle.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
    struct AnimationViewportState {
        uint32_t width = 0;
        uint32_t height = 0;

        TextureHandle textureId = InvalidTextureHandle;
        std::string currentGroup = RenderTargetGroupNames::Main;
        std::string currentTarget = RenderTargetNames::Main;
        std::string currentView = Vk::ImageViewNames::Default;

        bool isHovered = false;
        bool isFocused = false;

        ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
        bool useSnap = false;

        glm::vec3 snapTranslate{ 1.0f, 1.0f, 1.0f };
        float snapAngle = 45.0f;
        float snapScale = 0.5f;

        uint32_t activeEntity = 0;
        uint32_t activeAnimationId = 0xFFFFFFFF;

        glm::mat4 cameraView{ 1.0f };
        glm::mat4 cameraProj{ 1.0f };
        glm::mat4 entityWorldTransform{ 1.0f };

        bool enableDebugVisibility = false;
        uint32_t debugVisibilityMode = 0;
    };
}