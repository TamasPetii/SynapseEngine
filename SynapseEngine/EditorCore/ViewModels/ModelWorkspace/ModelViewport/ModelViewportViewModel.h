#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ModelViewportState.h"
#include "ModelViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/ISettingsApi.h"
#include "EditorCore/Api/IModelApi.h"

namespace Syn {
    class ModelViewportViewModel : public IViewModel<ModelViewportState, ModelViewportIntent> {
    public:
        ModelViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IModelApi* modelApi);
        ~ModelViewportViewModel() override = default;

        const ModelViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const ModelViewportIntent& intent) override;
    private:
        void HandlePickMesh(const PickMeshIntent& intent);
        void HandleResize(const ResizeModelViewportIntent& intent);
        void HandleChangeTarget(const ChangeModelTargetIntent& intent);
        void HandleGizmoTransform(const ApplyModelGizmoTransformIntent& intent);
        void HandleToggleDebugVisibility(const ToggleModelDebugVisibilityIntent& intent);
        void HandleChangeDebugVisibilityMode(const ChangeModelDebugVisibilityModeIntent& intent);
    private:
        IRenderApi* _renderApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        ISettingsApi* _settingsApi = nullptr;
        IModelApi* _modelApi = nullptr;
        ModelViewportState _state;
    };
}