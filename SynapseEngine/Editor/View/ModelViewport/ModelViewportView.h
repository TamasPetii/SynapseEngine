#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/ModelViewport/ModelViewportViewModel.h"
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