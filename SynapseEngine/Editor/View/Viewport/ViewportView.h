#pragma once

#include "../IView.h"
#include "EditorCore/ViewModels/Viewport/ViewportViewModel.h"
#include <imgui.h>

namespace Syn {
    class ViewportView : public IView<ViewportViewModel> {
    public:
        void Draw(ViewportViewModel& vm) override;
    private:
        void RenderFloatingToolbar(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size);
        void RenderSimulationToolbar(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size);

        void DrawGizmoPopup(ViewportViewModel& vm, const ViewportState& state);
        void DrawImagePopup(ViewportViewModel& vm, const ViewportState& state);
        void DrawDebugPopup(ViewportViewModel& vm, const ViewportState& state);

        void DrawGizmo(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size);
        void HandleShortcuts(ViewportViewModel& vm);
    };
}