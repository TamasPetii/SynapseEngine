#pragma once
#include "Engine/SynApi.h"
#include "IAnimationProcessor.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API IAnimationProcessorPipeline
    {
    public:
        virtual void AddProcessor(std::unique_ptr<IAnimationProcessor> processor) = 0;
        virtual void Run(CookedAnimation& inOutAnimation, const CookedModel& baseModel) = 0;
    };
}