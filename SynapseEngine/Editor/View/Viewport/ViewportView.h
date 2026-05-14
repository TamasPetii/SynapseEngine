#pragma once

#include "../IView.h"
#include "EditorCore/ViewModels/Viewport/ViewportViewModel.h"
#include <imgui.h>

namespace Syn {
    class ViewportView : public IView<ViewportViewModel> {
    public:
        void Draw(ViewportViewModel& vm) override;
    private:
        void DrawGizmoMenu(ViewportViewModel& vm, const ViewportState& state);
        void DrawImageMenu(ViewportViewModel& vm, const ViewportState& state);
        void DrawDebugMenu(ViewportViewModel& vm, const ViewportState& state);
        void DrawGizmo(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size);
        void HandleShortcuts(ViewportViewModel& vm);
    };
}