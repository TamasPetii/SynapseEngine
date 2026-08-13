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
#include "EditorCore/Api/IAudioSourceApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class AudioSourceApiImpl : public IAudioSourceApi {
    public:
        AudioSourceApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasAudioSource(EntityID entity) const override;

        uint32_t GetAudioSourceSoundIndex(EntityID entity) const override;
        bool GetAudioSourcePlay(EntityID entity) const override;
        bool GetAudioSourceLoop(EntityID entity) const override;
        bool GetAudioSourceIsSpatialized(EntityID entity) const override;
        float GetAudioSourceVolume(EntityID entity) const override;
        float GetAudioSourcePitch(EntityID entity) const override;
        float GetAudioSourceMinDistance(EntityID entity) const override;
        float GetAudioSourceMaxDistance(EntityID entity) const override;

        void SetAudioSourceSoundIndex(EntityID entity, uint32_t soundIndex) override;
        void SetAudioSourcePlay(EntityID entity, bool play) override;
        void SetAudioSourceLoop(EntityID entity, bool loop) override;
        void SetAudioSourceIsSpatialized(EntityID entity, bool isSpatialized) override;
        void SetAudioSourceVolume(EntityID entity, float volume) override;
        void SetAudioSourcePitch(EntityID entity, float pitch) override;
        void SetAudioSourceMinDistance(EntityID entity, float minDistance) override;
        void SetAudioSourceMaxDistance(EntityID entity, float maxDistance) override;

    private:
        SceneManager* _sceneManager;
    };
}