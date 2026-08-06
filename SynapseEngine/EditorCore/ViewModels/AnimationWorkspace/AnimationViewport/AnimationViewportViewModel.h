#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "AnimationViewportState.h"
#include "AnimationViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/ISettingsApi.h"
#include "EditorCore/Api/IAnimationApi.h"

namespace Syn {
    class AnimationViewportViewModel : public IViewModel<AnimationViewportState, AnimationViewportIntent> {
    public:
        AnimationViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IAnimationApi* animationApi);
        ~AnimationViewportViewModel() override = default;

        const AnimationViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const AnimationViewportIntent& intent) override;
    private:
        void HandlePickMesh(const PickAnimationMeshIntent& intent);
        void HandleResize(const ResizeAnimationViewportIntent& intent);
        void HandleChangeTarget(const ChangeAnimationTargetIntent& intent);
        void HandleGizmoTransform(const ApplyAnimationGizmoTransformIntent& intent);
        void HandleToggleDebugVisibility(const ToggleAnimationDebugVisibilityIntent& intent);
        void HandleChangeDebugVisibilityMode(const ChangeAnimationDebugVisibilityModeIntent& intent);
    private:
        IRenderApi* _renderApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        ISettingsApi* _settingsApi = nullptr;
        IAnimationApi* _animationApi = nullptr;
        AnimationViewportState _state;
    };
}