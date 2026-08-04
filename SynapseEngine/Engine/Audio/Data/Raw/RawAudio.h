#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    struct SYN_API RawAudio {
        uint32_t channels = 0;
        uint32_t sampleRate = 0;
        uint64_t totalFrames = 0;
        std::vector<float> samples;
    };
}