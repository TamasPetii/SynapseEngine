#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "AnimationSequencerState.h"
#include "AnimationSequencerIntent.h"
#include "EditorCore/Api/IAnimationApi.h"

namespace Syn {
    class AnimationSequencerViewModel : public IViewModel<AnimationSequencerState, AnimationSequencerIntent> {
    public:
        AnimationSequencerViewModel(IAnimationApi* animationApi);
        ~AnimationSequencerViewModel() override = default;

        const AnimationSequencerState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const AnimationSequencerIntent& intent) override;

    private:
        IAnimationApi* _animationApi = nullptr;
        AnimationSequencerState _state;
    };
}