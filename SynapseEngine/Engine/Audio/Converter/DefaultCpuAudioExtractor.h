#pragma once
#include "Engine/SynApi.h"
#include "ICpuAudioExtractor.h"

namespace Syn
{
    class SYN_API DefaultCpuAudioExtractor : public ICpuAudioExtractor
    {
    public:
        DefaultCpuAudioExtractor() = default;
        ~DefaultCpuAudioExtractor() override = default;

        void Extract(const CookedAudio& cookedData, CpuAudioData& outCpuData) const override;
    };
}