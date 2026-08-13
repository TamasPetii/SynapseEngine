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