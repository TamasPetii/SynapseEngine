// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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