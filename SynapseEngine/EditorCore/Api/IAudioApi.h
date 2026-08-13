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
#include <cstdint>
#include <vector>
#include <string>
#include "IApi.h"
#include "Engine/Audio/Data/Cpu/CpuAudioData.h"

namespace Syn
{
    constexpr uint32_t INVALID_AUDIO_ID = 0xFFFFFFFF;

    struct AudioItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IAudioApi : public IApi {
    public:
        virtual ~IAudioApi() = default;

        virtual std::vector<AudioItemData> GetAllAudios() const = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual void SetSelected(uint32_t audioId) = 0;
        virtual uint32_t GetSelected() const = 0;

        virtual const CpuAudioData* GetAudioCpuData(uint32_t audioId) const = 0;

        virtual void Play() = 0;
        virtual void Pause() = 0;
        virtual void Stop() = 0;
        virtual void SetPlaybackTime(float timeInSeconds) = 0;

        virtual bool IsPlaying() const = 0;
        virtual float GetPlaybackTime() const = 0;
        virtual float GetDuration() const = 0;
    };
}