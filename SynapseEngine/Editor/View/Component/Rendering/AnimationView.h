#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Rendering/Animation/AnimationViewModel.h"

namespace Syn {
    class AnimationView : public IView<AnimationViewModel> {
    public:
        void Draw(AnimationViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}