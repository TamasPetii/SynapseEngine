#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    struct AudioWaveformChunk {
        float minAmp = 0.0f;
        float maxAmp = 0.0f;
    };
}