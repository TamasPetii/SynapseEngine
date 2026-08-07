#include "AudioViewportViewModel.h"

namespace Syn {

    AudioViewportViewModel::AudioViewportViewModel(IAudioApi* audioApi)
        : _audioApi(audioApi) {}

    void AudioViewportViewModel::SyncWithEngine() {
        if (!_audioApi) return;

        uint32_t selectedId = _audioApi->GetSelected();

        if (_state.activeAudioId != selectedId) {
            _state.activeAudioId = selectedId;
            _audioApi->Stop();

            if (_state.activeAudioId != 0xFFFFFFFF) {
                _state.currentAudioData = _audioApi->GetAudioCpuData(_state.activeAudioId);
                _state.duration = _audioApi->GetDuration();
            }
            else {
                _state.currentAudioData = nullptr;
                _state.duration = 0.0f;
            }
        }

        _state.isPlaying = _audioApi->IsPlaying();
        _state.currentTime = _audioApi->GetPlaybackTime();
    }

    void AudioViewportViewModel::Dispatch(const AudioViewportIntent& intent) {
        if (!_audioApi || _state.activeAudioId == 0xFFFFFFFF) return;

        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, AudioViewportPlayIntent>) {
                _audioApi->Play();
            }
            else if constexpr (std::is_same_v<T, AudioViewportPauseIntent>) {
                _audioApi->Pause();
            }
            else if constexpr (std::is_same_v<T, AudioViewportStopIntent>) {
                _audioApi->Stop();
            }
            else if constexpr (std::is_same_v<T, AudioViewportSeekIntent>) {
                _audioApi->SetPlaybackTime(arg.timeInSeconds);
            }
            }, intent);
    }
}