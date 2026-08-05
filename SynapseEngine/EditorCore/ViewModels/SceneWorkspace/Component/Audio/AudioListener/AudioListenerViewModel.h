#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "AudioListenerState.h"
#include "AudioListenerIntent.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IAudioListenerApi.h"

namespace Syn {
    class AudioListenerViewModel : public IViewModel<AudioListenerState, AudioListenerIntent> {
    public:
        AudioListenerViewModel(ISelectionApi* selectionApi, IAudioListenerApi* audioListenerApi);
        ~AudioListenerViewModel() override = default;

        const AudioListenerState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const AudioListenerIntent& intent) override;

    private:
        void HandleSetActive(const SetAudioListenerActiveIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IAudioListenerApi* _audioListenerApi = nullptr;
        AudioListenerState _state;
    };
}