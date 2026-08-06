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