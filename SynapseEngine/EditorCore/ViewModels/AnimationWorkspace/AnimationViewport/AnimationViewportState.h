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