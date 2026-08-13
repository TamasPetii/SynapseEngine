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

#pragma once
#include "EditorCore/Api/IAudioApi.h"
#include "Engine/Audio/AudioManager.h"

namespace Syn {
    class AudioApiImpl : public IAudioApi {
    public:
        AudioApiImpl(AudioManager* audioManager)
            : _audioManager(audioManager) {}

        std::vector<AudioItemData> GetAllAudios() const override;
        uint64_t GetVersion() const override;

        void SetSelected(uint32_t audioId) override;
        uint32_t GetSelected() const override;

        const CpuAudioData* GetAudioCpuData(uint32_t audioId) const override;

        void Play() override;
        void Pause() override;
        void Stop() override;
        void SetPlaybackTime(float timeInSeconds) override;

        bool IsPlaying() const override;
        float GetPlaybackTime() const override;
        float GetDuration() const override;
    private:
        AudioManager* _audioManager;
        uint32_t _selectedAudioId = INVALID_AUDIO_ID;
    };
}