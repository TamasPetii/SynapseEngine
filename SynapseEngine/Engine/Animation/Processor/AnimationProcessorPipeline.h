#pragma once
#include "Engine/SynApi.h"
#include "IAnimationProcessorPipeline.h"

namespace Syn
{
    class SYN_API AnimationProcessorPipeline : public IAnimationProcessorPipeline
    {
    public:
        AnimationProcessorPipeline() = default;

        AnimationProcessorPipeline(const AnimationProcessorPipeline&) = delete;
        AnimationProcessorPipeline& operator=(const AnimationProcessorPipeline&) = delete;

        virtual void AddProcessor(std::unique_ptr<IAnimationProcessor> processor) override;
        virtual void Run(CookedAnimation& inOutAnimation, const CookedModel& baseModel) override;
    private:
        std::vector<std::unique_ptr<IAnimationProcessor>> _processors;
    };
}