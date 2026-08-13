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