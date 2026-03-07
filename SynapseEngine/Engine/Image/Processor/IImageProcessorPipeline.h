#pragma once
#include "Engine/SynApi.h"
#include "IImageProcessor.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API IImageProcessorPipeline
    {
    public:
        virtual ~IImageProcessorPipeline() = default;
        virtual void AddProcessor(std::unique_ptr<IImageProcessor> processor) = 0;
        virtual void Run(CookedImage& cookedImage) = 0;
    };
}