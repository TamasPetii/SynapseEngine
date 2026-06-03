#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ViewportState.h"
#include "ViewportIntent.h"
#include "EditorCore/API/IRenderAPI.h"
#include "EditorCore/API/ISelectionAPI.h"
#include "EditorCore/API/ITransformAPI.h"
#include "EditorCore/API/ISettingsAPI.h"

namespace Syn {
    class ViewportViewModel : public IViewModel<ViewportState, ViewportIntent> {
    public:
        ViewportViewModel(IRenderAPI* renderApi, ISelectionAPI* selectionApi, ITransformAPI* transformApi, ISettingsAPI* settingsApi);
        ~ViewportViewModel() override = default;

        const ViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const ViewportIntent& intent) override;

    private:
        void HandlePickEntity(const PickEntityIntent& intent);
        void HandleResize(const ResizeViewportIntent& intent);
        void HandleChangeTarget(const ChangeTargetIntent& intent);
        void HandleGizmoTransform(const ApplyGizmoTransformIntent& intent);
        void HandleToggleDebugVisibility(const ToggleDebugVisibilityIntent& intent);
        void HandleChangeDebugVisibilityMode(const ChangeDebugVisibilityModeIntent& intent);

    private:
        IRenderAPI* _renderApi = nullptr;
        ISelectionAPI* _selectionApi = nullptr;
        ITransformAPI* _transformApi = nullptr;
        ISettingsAPI* _settingsApi = nullptr;
        ViewportState _state;
    };
}