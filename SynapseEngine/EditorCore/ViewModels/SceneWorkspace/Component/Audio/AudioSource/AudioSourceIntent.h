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