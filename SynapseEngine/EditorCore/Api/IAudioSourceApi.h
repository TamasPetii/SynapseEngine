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
#include "EditorCore/Types/EntityHandle.h"
#include "IApi.h"
#include <cstdint>

namespace Syn {
    class IAudioSourceApi : public IApi {
    public:
        virtual ~IAudioSourceApi() = default;

        virtual bool HasAudioSource(EntityID entity) const = 0;

        virtual uint32_t GetAudioSourceSoundIndex(EntityID entity) const = 0;
        virtual bool GetAudioSourcePlay(EntityID entity) const = 0;
        virtual bool GetAudioSourceLoop(EntityID entity) const = 0;
        virtual bool GetAudioSourceIsSpatialized(EntityID entity) const = 0;
        virtual float GetAudioSourceVolume(EntityID entity) const = 0;
        virtual float GetAudioSourcePitch(EntityID entity) const = 0;
        virtual float GetAudioSourceMinDistance(EntityID entity) const = 0;
        virtual float GetAudioSourceMaxDistance(EntityID entity) const = 0;

        virtual void SetAudioSourceSoundIndex(EntityID entity, uint32_t soundIndex) = 0;
        virtual void SetAudioSourcePlay(EntityID entity, bool play) = 0;
        virtual void SetAudioSourceLoop(EntityID entity, bool loop) = 0;
        virtual void SetAudioSourceIsSpatialized(EntityID entity, bool isSpatialized) = 0;
        virtual void SetAudioSourceVolume(EntityID entity, float volume) = 0;
        virtual void SetAudioSourcePitch(EntityID entity, float pitch) = 0;
        virtual void SetAudioSourceMinDistance(EntityID entity, float minDistance) = 0;
        virtual void SetAudioSourceMaxDistance(EntityID entity, float maxDistance) = 0;
    };
}