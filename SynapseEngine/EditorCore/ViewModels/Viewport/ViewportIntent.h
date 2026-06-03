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

    struct ChangeSnapTranslateIntent {
        glm::vec3 snap;
    };

    struct ChangeSnapRotateIntent {
        float angle;
    };

    struct ChangeSnapScaleIntent {
        float scale;
    };

    struct ApplyGizmoTransformIntent { 
        glm::mat4 newWorldMatrix;
    }; 

    struct PickEntityIntent {
        uint32_t x;
        uint32_t y;
    };

    struct ToggleDebugVisibilityIntent { 
        bool enabled;
    };

    struct ChangeDebugVisibilityModeIntent { 
        uint32_t mode;
    };

    struct PlaySimulationIntent {};
    struct PauseSimulationIntent {};
    struct StopSimulationIntent {};

    using ViewportIntent = std::variant<
        ResizeViewportIntent,
        ChangeTargetIntent,
        ChangeGizmoOperationIntent,
        ChangeGizmoModeIntent,
        ToggleSnapIntent,
        ApplyGizmoTransformIntent,
        PickEntityIntent,
        ToggleDebugVisibilityIntent,
        ChangeDebugVisibilityModeIntent,
        ChangeSnapTranslateIntent,
        ChangeSnapRotateIntent,
        ChangeSnapScaleIntent,
        PlaySimulationIntent,
        PauseSimulationIntent,
        StopSimulationIntent
    >;
}