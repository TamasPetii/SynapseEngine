#pragma once
#include "Engine/SynApi.h"
#include "IAudioProcessor.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API IAudioProcessorPipeline
    {
    public:
        virtual ~IAudioProcessorPipeline() = default;
        virtual void AddProcessor(std::unique_ptr<IAudioProcessor> processor) = 0;
        virtual void Run(CookedAudio& cookedAudio) = 0;
    };
}