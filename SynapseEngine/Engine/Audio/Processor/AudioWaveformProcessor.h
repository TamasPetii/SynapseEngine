#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Processor/IAudioProcessor.h"

namespace Syn
{
    class SYN_API AudioWaveformProcessor : public IAudioProcessor
    {
    public:
        AudioWaveformProcessor(uint32_t resolution = 512);
        ~AudioWaveformProcessor() override = default;

        void Process(CookedAudio& cookedAudio) override;
    private:
        uint32_t _resolution;
    };
}