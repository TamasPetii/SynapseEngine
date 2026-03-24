#pragma once
#include <string>
#include <variant>
#include <cstdint>
#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace Syn {
    struct ResizeViewportIntent {
        uint32_t width;
        uint32_t height;
    };

    struct ChangeTargetIntent {
        std::string currentGroup;
        std::string targetName; 
        std::string viewName;
    };

    struct ChangeGizmoOperationIntent { 
        ImGuizmo::OPERATION op;
    };

    struct ChangeGizmoModeIntent { 
        ImGuizmo::MODE mode;
    };

    struct ToggleSnapIntent { 
        bool useSnap;
    };

    struct ApplyGizmoTransformIntent { 
        glm::mat4 newWorldMatrix;
    }; 

    using ViewportIntent = std::variant<
        ResizeViewportIntent,
        ChangeTargetIntent,
        ChangeGizmoOperationIntent,
        ChangeGizmoModeIntent,
        ToggleSnapIntent,
        ApplyGizmoTransformIntent
    >;
}