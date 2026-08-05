#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Audio/AudioListener/AudioListenerViewModel.h"

namespace Syn {
    class AudioListenerView : public IView<AudioListenerViewModel> {
    public:
        void Draw(AudioListenerViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}