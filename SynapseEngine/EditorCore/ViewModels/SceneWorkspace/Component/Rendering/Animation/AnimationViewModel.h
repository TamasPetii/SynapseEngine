#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "AnimationState.h"
#include "AnimationIntent.h"
#include "AnimationCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IAnimationCompApi.h"

namespace Syn {
    class AnimationViewModel : public IViewModel<AnimationState, AnimationIntent> {
    public:
        AnimationViewModel(ISelectionApi* selectionApi, IAnimationCompApi* animApi);
        ~AnimationViewModel() override = default;

        const AnimationState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const AnimationIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        IAnimationCompApi* _animApi = nullptr;
        AnimationState _state;

        DragInteraction<float> _speedDrag;
    };
}