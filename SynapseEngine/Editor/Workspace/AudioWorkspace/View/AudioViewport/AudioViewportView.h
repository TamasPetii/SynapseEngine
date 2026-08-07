#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/AudioWorkspace/AudioViewport/AudioViewportViewModel.h"

namespace Syn {
    class AudioViewportView : public IView<AudioViewportViewModel> {
    public:
        void Draw(AudioViewportViewModel& vm) override;
    private:
        bool _cardState = true;
    };
}