#pragma once
#include <string>
#include <variant>
#include <cstdint>
#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace Syn {
    struct ResizeModelViewportIntent { uint32_t width; uint32_t height; };
    struct ChangeModelTargetIntent { std::string currentGroup; std::string targetName; std::string viewName; };

    struct ChangeModelGizmoOperationIntent { ImGuizmo::OPERATION op; };
    struct ChangeModelGizmoModeIntent { ImGuizmo::MODE mode; };
    struct ToggleModelSnapIntent { bool useSnap; };
    struct ChangeModelSnapTranslateIntent { glm::vec3 snap; };
    struct ChangeModelSnapRotateIntent { float angle; };
    struct ChangeModelSnapScaleIntent { float scale; };
    struct ApplyModelGizmoTransformIntent { glm::mat4 newWorldMatrix; };

    struct PickMeshIntent { uint32_t x; uint32_t y; };

    struct ToggleModelDebugVisibilityIntent { bool enabled; };
    struct ChangeModelDebugVisibilityModeIntent { uint32_t mode; };

    using ModelViewportIntent = std::variant<
        ResizeModelViewportIntent,
        ChangeModelTargetIntent,
        ChangeModelGizmoOperationIntent,
        ChangeModelGizmoModeIntent,
        ToggleModelSnapIntent,
        ApplyModelGizmoTransformIntent,
        PickMeshIntent,
        ToggleModelDebugVisibilityIntent,
        ChangeModelDebugVisibilityModeIntent,
        ChangeModelSnapTranslateIntent,
        ChangeModelSnapRotateIntent,
        ChangeModelSnapScaleIntent
    >;
}