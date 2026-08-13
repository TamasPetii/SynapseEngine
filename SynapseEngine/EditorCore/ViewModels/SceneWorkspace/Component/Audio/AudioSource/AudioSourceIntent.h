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
#include <variant>
#include <cstdint>

namespace Syn
{
    struct SetAudioSourceSoundIndexIntent {
        uint32_t soundIndex;
    };

    struct SetAudioSourcePlayIntent {
        bool play;
    };

    struct SetAudioSourceLoopIntent {
        bool loop;
    };

    struct SetAudioSourceIsSpatializedIntent {
        bool isSpatialized;
    };

    struct SetAudioSourceVolumeIntent {
        float volume;
        bool isDragging;
    };

    struct SetAudioSourcePitchIntent {
        float pitch;
        bool isDragging;
    };

    struct SetAudioSourceMinDistanceIntent {
        float minDistance;
        bool isDragging;
    };

    struct SetAudioSourceMaxDistanceIntent {
        float maxDistance;
        bool isDragging;
    };

    using AudioSourceIntent = std::variant<
        SetAudioSourceSoundIndexIntent,
        SetAudioSourcePlayIntent,
        SetAudioSourceLoopIntent,
        SetAudioSourceIsSpatializedIntent,
        SetAudioSourceVolumeIntent,
        SetAudioSourcePitchIntent,
        SetAudioSourceMinDistanceIntent,
        SetAudioSourceMaxDistanceIntent>;
}