#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Audio/AudioSource/AudioSourceViewModel.h"

namespace Syn {
    class AudioSourceView : public IView<AudioSourceViewModel> {
    public:
        void Draw(AudioSourceViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}