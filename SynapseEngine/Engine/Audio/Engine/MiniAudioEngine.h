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
#include "IAudioEngine.h"
#include <miniaudio.h>
#include <unordered_map>
#include <mutex>

namespace Syn
{
    class SYN_API MiniAudioEngine : public IAudioEngine
    {
    public:
        MiniAudioEngine() = default;
        ~MiniAudioEngine() override;

        void Init() override;
        void Shutdown() override;

        void SetListenerTransform(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up) override;
        void UpdateSound(EntityID entity, const AudioSourceComponent& component, const CpuAudioData& audioData, const glm::vec3& position) override;
        void StopSound(EntityID entity) override;
        void StopAllSounds() override;

        void PlayPreview(const CpuAudioData* audioData) override;
        void PausePreview() override;
        void StopPreview() override;
        void SetPreviewTime(float time) override;
        bool IsPreviewPlaying() const override;
        float GetPreviewTime() const override;
    private:
        struct ActiveSound {
            ma_audio_buffer buffer;
            ma_sound sound;
            bool isInitialized = false;
        };

        ma_engine _engine;
        bool _isInitialized = false;

        std::unordered_map<EntityID, ActiveSound> _activeSounds;
        std::mutex _mutex;

        ActiveSound _previewSound;
        const CpuAudioData* _currentPreviewData = nullptr;
    };
}