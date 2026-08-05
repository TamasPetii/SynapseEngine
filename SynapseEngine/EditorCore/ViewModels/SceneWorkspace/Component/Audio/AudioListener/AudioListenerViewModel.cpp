#include "AudioListenerViewModel.h"

namespace Syn
{
    AudioListenerViewModel::AudioListenerViewModel(ISelectionApi* selectionApi, IAudioListenerApi* audioListenerApi)
        : _selectionApi(selectionApi), _audioListenerApi(audioListenerApi)
    {}

    const AudioListenerState& AudioListenerViewModel::GetState() const
    {
        return _state;
    }

    void AudioListenerViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_audioListenerApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _audioListenerApi->HasAudioListener(activeEntity))
        {
            _state.hasComponent = true;
            _state.active = _audioListenerApi->GetAudioListenerActive(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void AudioListenerViewModel::Dispatch(const AudioListenerIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetAudioListenerActiveIntent>) HandleSetActive(arg);
            }, intent);
    }

    void AudioListenerViewModel::HandleSetActive(const SetAudioListenerActiveIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.active = intent.active;
        _audioListenerApi->SetAudioListenerActive(activeEntity, intent.active);
    }
}