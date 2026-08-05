#pragma once
#include "Engine/SynApi.h"
#include "IShaderProcessor.h"
#include <vector>
#include <memory>

namespace Syn {
    class SYN_API IShaderProcessorPipeline {
    public:
        virtual ~IShaderProcessorPipeline() = default;
        virtual void AddProcessor(std::unique_ptr<IShaderProcessor> processor) = 0;
        virtual void Run(CookedShader& cookedShader) = 0;
    };
}