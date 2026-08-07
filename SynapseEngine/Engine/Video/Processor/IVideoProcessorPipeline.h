#pragma once
#include "Engine/SynApi.h"
#include "IVideoProcessor.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API IVideoProcessorPipeline
    {
    public:
        virtual ~IVideoProcessorPipeline() = default;
        virtual void AddProcessor(std::unique_ptr<IVideoProcessor> processor) = 0;
        virtual void Run(CookedVideoPacket& cookedPacket) = 0;
    };
}