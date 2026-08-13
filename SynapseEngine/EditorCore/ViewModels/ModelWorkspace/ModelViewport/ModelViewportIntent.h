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