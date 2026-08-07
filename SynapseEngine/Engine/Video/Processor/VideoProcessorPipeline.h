#pragma once
#include "Engine/SynApi.h"
#include "IVideoProcessorPipeline.h"

namespace Syn
{
    class SYN_API VideoProcessorPipeline : public IVideoProcessorPipeline
    {
    public:
        VideoProcessorPipeline() = default;

        VideoProcessorPipeline(const VideoProcessorPipeline&) = delete;
        VideoProcessorPipeline& operator=(const VideoProcessorPipeline&) = delete;

        void AddProcessor(std::unique_ptr<IVideoProcessor> processor) override;
        void Run(CookedVideoPacket& cookedPacket) override;
    private:
        std::vector<std::unique_ptr<IVideoProcessor>> _processors;
    };
}