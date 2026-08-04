#include "DefaultAudioProcessorPipeline.h"

namespace Syn
{
    void DefaultAudioProcessorPipeline::AddProcessor(std::unique_ptr<IAudioProcessor> processor)
    {
        _processors.push_back(std::move(processor));
    }

    void DefaultAudioProcessorPipeline::Run(CookedAudio& cookedAudio)
    {
        for (auto& processor : _processors)
        {
            processor->Process(cookedAudio);
        }
    }
}