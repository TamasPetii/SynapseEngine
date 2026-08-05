#pragma once
#include <cstdint>

namespace Syn {
    struct AudioSourceState {
        bool hasComponent = false;

        uint32_t soundIndex;
        bool play;
        bool loop;
        bool isSpatialized;
        float volume;
        float pitch;
        float minDistance;
        float maxDistance;
    };
}