#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Cooked/CookedAudio.h"
#include "Engine/Audio/Data/Cpu/CpuAudioData.h"

namespace Syn
{
    class SYN_API ICpuAudioExtractor
    {
    public:
        virtual ~ICpuAudioExtractor() = default;
        virtual void Extract(const CookedAudio& cookedData, CpuAudioData& outCpuData) const = 0;
    };
}