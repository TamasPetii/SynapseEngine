#include "VideoProcessorPipeline.h"

namespace Syn
{
    void VideoProcessorPipeline::AddProcessor(std::unique_ptr<IVideoProcessor> processor)
    {
        _processors.push_back(std::move(processor));
    }

    void VideoProcessorPipeline::Run(CookedVideoPacket& cookedPacket)
    {
        for (auto& processor : _processors)
        {
            processor->Process(cookedPacket);
        }
    }
}