#include "AnimationProcessorPipeline.h"

namespace Syn
{
    void AnimationProcessorPipeline::AddProcessor(std::unique_ptr<IAnimationProcessor> processor)
    {
        _processors.push_back(std::move(processor));
    }

    void AnimationProcessorPipeline::Run(CookedAnimation& inOutAnimation, const CookedModel& baseModel)
    {
        for (auto& processor : _processors)
        {
            processor->Process(inOutAnimation, baseModel);
        }
    }
}