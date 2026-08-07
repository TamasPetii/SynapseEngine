#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "AudioViewportState.h"
#include "AudioViewportIntent.h"
#include "EditorCore/Api/IAudioApi.h"

namespace Syn {
    class AudioViewportViewModel : public IViewModel<AudioViewportState, AudioViewportIntent> {
    public:
        AudioViewportViewModel(IAudioApi* audioApi);
        ~AudioViewportViewModel() override = default;

        const AudioViewportState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const AudioViewportIntent& intent) override;

    private:
        IAudioApi* _audioApi = nullptr;
        AudioViewportState _state;
    };
}