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
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/AnimationWorkspace/AnimationViewport/AnimationViewportViewModel.h"
#include <imgui.h>

namespace Syn {
    class AnimationViewportView : public IView<AnimationViewportViewModel> {
    public:
        void Draw(AnimationViewportViewModel& vm) override;
    private:
        void RenderFloatingToolbar(AnimationViewportViewModel& vm, const AnimationViewportState& state, ImVec2 startPos, ImVec2 size);

        void DrawGizmoPopup(AnimationViewportViewModel& vm, const AnimationViewportState& state);
        void DrawImagePopup(AnimationViewportViewModel& vm, const AnimationViewportState& state);
        void DrawDebugPopup(AnimationViewportViewModel& vm, const AnimationViewportState& state);

        void DrawGizmo(AnimationViewportViewModel& vm, const AnimationViewportState& state, ImVec2 startPos, ImVec2 size);
        void HandleShortcuts(AnimationViewportViewModel& vm);
    };
}