#pragma once
#include "Engine/SynApi.h"
#include "ICpuModelProcessor.h"
#include <memory>

namespace Syn
{
    class SYN_API ICpuModelProcessorPipeline
    {
    public:
        virtual ~ICpuModelProcessorPipeline() = default;
        virtual void AddProcessor(std::unique_ptr<ICpuModelProcessor> processor) = 0;
        virtual void Run(CpuModelData& cpuData) = 0;
    };
}