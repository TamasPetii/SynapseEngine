#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Cooked/CookedAudio.h"
#include "Engine/Audio/Data/Cpu/CpuAudioData.h"
#include <memory>

namespace Syn
{
    struct SYN_API Sound {
        CpuAudioData cpuData;
        std::unique_ptr<CookedAudio> transientCpuData;
    };
}