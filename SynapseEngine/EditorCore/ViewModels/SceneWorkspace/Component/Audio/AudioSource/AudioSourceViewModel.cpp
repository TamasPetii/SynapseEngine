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

#include "AudioSourceViewModel.h"

namespace Syn
{
    AudioSourceViewModel::AudioSourceViewModel(ISelectionApi* selectionApi, IAudioSourceApi* audioSourceApi)
        : _selectionApi(selectionApi), _audioSourceApi(audioSourceApi)
    {}

    const AudioSourceState& AudioSourceViewModel::GetState() const
    {
        return _state;
    }

    void AudioSourceViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_audioSourceApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _audioSourceApi->HasAudioSource(activeEntity))
        {
            _state.hasComponent = true;

            _state.soundIndex = _audioSourceApi->GetAudioSourceSoundIndex(activeEntity);
            _state.play = _audioSourceApi->GetAudioSourcePlay(activeEntity);
            _state.loop = _audioSourceApi->GetAudioSourceLoop(activeEntity);
            _state.isSpatialized = _audioSourceApi->GetAudioSourceIsSpatialized(activeEntity);

            if (!_volumeDrag.IsDragging())
                _state.volume = _audioSourceApi->GetAudioSourceVolume(activeEntity);
            if (!_pitchDrag.IsDragging())
                _state.pitch = _audioSourceApi->GetAudioSourcePitch(activeEntity);
            if (!_minDistanceDrag.IsDragging())
                _state.minDistance = _audioSourceApi->GetAudioSourceMinDistance(activeEntity);
            if (!_maxDistanceDrag.IsDragging())
                _state.maxDistance = _audioSourceApi->GetAudioSourceMaxDistance(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void AudioSourceViewModel::Dispatch(const AudioSourceIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetAudioSourceSoundIndexIntent>)         HandleSetSoundIndex(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourcePlayIntent>)          HandleSetPlay(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourceLoopIntent>)          HandleSetLoop(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourceIsSpatializedIntent>) HandleSetIsSpatialized(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourceVolumeIntent>)        HandleSetVolume(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourcePitchIntent>)         HandleSetPitch(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourceMinDistanceIntent>)   HandleSetMinDistance(arg);
                else if constexpr (std::is_same_v<T, SetAudioSourceMaxDistanceIntent>)   HandleSetMaxDistance(arg); }, intent);
    }

    void AudioSourceViewModel::HandleSetSoundIndex(const SetAudioSourceSoundIndexIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.soundIndex = intent.soundIndex;
        _audioSourceApi->SetAudioSourceSoundIndex(activeEntity, intent.soundIndex);
    }

    void AudioSourceViewModel::HandleSetPlay(const SetAudioSourcePlayIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.play = intent.play;
        _audioSourceApi->SetAudioSourcePlay(activeEntity, intent.play);
    }

    void AudioSourceViewModel::HandleSetLoop(const SetAudioSourceLoopIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.loop = intent.loop;
        _audioSourceApi->SetAudioSourceLoop(activeEntity, intent.loop);
    }

    void AudioSourceViewModel::HandleSetIsSpatialized(const SetAudioSourceIsSpatializedIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.isSpatialized = intent.isSpatialized;
        _audioSourceApi->SetAudioSourceIsSpatialized(activeEntity, intent.isSpatialized);
    }

    void AudioSourceViewModel::HandleSetVolume(const SetAudioSourceVolumeIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _volumeDrag.Handle(intent.isDragging, intent.volume, _state.volume,
            [&](const float& v) { _audioSourceApi->SetAudioSourceVolume(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeAudioSourceVolumeCommand>(_audioSourceApi, activeEntity, s, e); });
    }

    void AudioSourceViewModel::HandleSetPitch(const SetAudioSourcePitchIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _pitchDrag.Handle(intent.isDragging, intent.pitch, _state.pitch,
            [&](const float& v) { _audioSourceApi->SetAudioSourcePitch(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeAudioSourcePitchCommand>(_audioSourceApi, activeEntity, s, e); });
    }

    void AudioSourceViewModel::HandleSetMinDistance(const SetAudioSourceMinDistanceIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _minDistanceDrag.Handle(intent.isDragging, intent.minDistance, _state.minDistance,
            [&](const float& v) { _audioSourceApi->SetAudioSourceMinDistance(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeAudioSourceMinDistanceCommand>(_audioSourceApi, activeEntity, s, e); });
    }

    void AudioSourceViewModel::HandleSetMaxDistance(const SetAudioSourceMaxDistanceIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _maxDistanceDrag.Handle(intent.isDragging, intent.maxDistance, _state.maxDistance,
            [&](const float& v) { _audioSourceApi->SetAudioSourceMaxDistance(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeAudioSourceMaxDistanceCommand>(_audioSourceApi, activeEntity, s, e); });
    }
}