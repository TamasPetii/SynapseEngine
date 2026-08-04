#pragma once
#include "Engine/SynApi.h"
#include "IAudioProcessorPipeline.h"

namespace Syn
{
    class SYN_API DefaultAudioProcessorPipeline : public IAudioProcessorPipeline
    {
    public:
        DefaultAudioProcessorPipeline() = default;

        DefaultAudioProcessorPipeline(const DefaultAudioProcessorPipeline&) = delete;
        DefaultAudioProcessorPipeline& operator=(const DefaultAudioProcessorPipeline&) = delete;

        void AddProcessor(std::unique_ptr<IAudioProcessor> processor) override;
        void Run(CookedAudio& cookedAudio) override;
    private:
        std::vector<std::unique_ptr<IAudioProcessor>> _processors;
    };
}