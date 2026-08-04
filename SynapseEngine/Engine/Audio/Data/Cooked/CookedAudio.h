#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    struct SYN_API CookedAudio {
        uint32_t channels = 0;
        uint32_t sampleRate = 0;
        uint64_t totalFrames = 0;

        bool isSpatialized = false;

        std::vector<float> samples;
    };
}