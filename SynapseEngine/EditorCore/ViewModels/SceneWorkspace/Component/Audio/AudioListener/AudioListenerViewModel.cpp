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