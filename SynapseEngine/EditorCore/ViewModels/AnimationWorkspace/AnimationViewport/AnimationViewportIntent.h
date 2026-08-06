#pragma once
#include <string>
#include <variant>
#include <cstdint>
#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace Syn {
    struct ResizeAnimationViewportIntent { uint32_t width; uint32_t height; };
    struct ChangeAnimationTargetIntent { std::string currentGroup; std::string targetName; std::string viewName; };
    struct ChangeAnimationGizmoOperationIntent { ImGuizmo::OPERATION op; };
    struct ChangeAnimationGizmoModeIntent { ImGuizmo::MODE mode; };
    struct ToggleAnimationSnapIntent { bool useSnap; };
    struct ChangeAnimationSnapTranslateIntent { glm::vec3 snap; };
    struct ChangeAnimationSnapRotateIntent { float angle; };
    struct ChangeAnimationSnapScaleIntent { float scale; };
    struct ApplyAnimationGizmoTransformIntent { glm::mat4 newWorldMatrix; };
    struct PickAnimationMeshIntent { uint32_t x; uint32_t y; };
    struct ToggleAnimationDebugVisibilityIntent { bool enabled; };
    struct ChangeAnimationDebugVisibilityModeIntent { uint32_t mode; };

    using AnimationViewportIntent = std::variant<
        ResizeAnimationViewportIntent,
        ChangeAnimationTargetIntent,
        ChangeAnimationGizmoOperationIntent,
        ChangeAnimationGizmoModeIntent,
        ToggleAnimationSnapIntent,
        ApplyAnimationGizmoTransformIntent,
        PickAnimationMeshIntent,
        ToggleAnimationDebugVisibilityIntent,
        ChangeAnimationDebugVisibilityModeIntent,
        ChangeAnimationSnapTranslateIntent,
        ChangeAnimationSnapRotateIntent,
        ChangeAnimationSnapScaleIntent
    >;
}