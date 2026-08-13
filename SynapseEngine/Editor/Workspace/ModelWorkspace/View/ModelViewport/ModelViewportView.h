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
#include "EditorCore/ViewModels/ModelWorkspace/ModelViewport/ModelViewportViewModel.h"
#include <imgui.h>

namespace Syn {
    class ModelViewportView : public IView<ModelViewportViewModel> {
    public:
        void Draw(ModelViewportViewModel& vm) override;
    private:
        void RenderFloatingToolbar(ModelViewportViewModel& vm, const ModelViewportState& state, ImVec2 startPos, ImVec2 size);

        void DrawGizmoPopup(ModelViewportViewModel& vm, const ModelViewportState& state);
        void DrawImagePopup(ModelViewportViewModel& vm, const ModelViewportState& state);
        void DrawDebugPopup(ModelViewportViewModel& vm, const ModelViewportState& state);

        void DrawGizmo(ModelViewportViewModel& vm, const ModelViewportState& state, ImVec2 startPos, ImVec2 size);
        void HandleShortcuts(ModelViewportViewModel& vm);
    };
}