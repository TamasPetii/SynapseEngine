#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Rendering/Animation/AnimationViewModel.h"

namespace Syn {
    class AnimationView : public IView<AnimationViewModel> {
    public:
        void Draw(AnimationViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}