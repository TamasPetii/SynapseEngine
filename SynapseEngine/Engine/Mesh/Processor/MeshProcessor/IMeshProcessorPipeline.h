#pragma once
#include "Engine/SynApi.h"
#include "IMeshProcessor.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API IMeshProcessorPipeline
    {
    public:
        virtual void AddProcessor(std::unique_ptr<IMeshProcessor> processor) = 0;
        virtual void Run(CookedModel& cookedModel) = 0;
    };
}