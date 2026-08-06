#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/AnimationWorkspace/AnimationSequencer/AnimationSequencerViewModel.h"

namespace Syn {
    class AnimationSequencerView : public IView<AnimationSequencerViewModel> {
    public:
        void Draw(AnimationSequencerViewModel& vm) override;
    private:
        bool _cardState = true;
    };
}