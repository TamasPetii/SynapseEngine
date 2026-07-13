#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ViewportState.h"
#include "ViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/ISettingsApi.h"
#include "EditorCore/Api/IHierarchyApi.h"

namespace Syn {
    class ViewportViewModel : public IViewModel<ViewportState, ViewportIntent> {
    public:
        ViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IHierarchyApi* hierarchyApi);
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
        IRenderApi* _renderApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        ISettingsApi* _settingsApi = nullptr;
        IHierarchyApi* _hierarchyApi = nullptr;
        ViewportState _state;
    };
}