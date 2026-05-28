#pragma once
#include "ICpuModelProcessorPipeline.h"
#include <vector>

namespace Syn
{
    class SYN_API CpuModelProcessorPipeline : public ICpuModelProcessorPipeline
    {
    public:
        CpuModelProcessorPipeline() = default;
        CpuModelProcessorPipeline(const CpuModelProcessorPipeline&) = delete;
        CpuModelProcessorPipeline& operator=(const CpuModelProcessorPipeline&) = delete;

        void AddProcessor(std::unique_ptr<ICpuModelProcessor> processor) override;
        void Run(CpuModelData& cpuData) override;
    private:
        std::vector<std::unique_ptr<ICpuModelProcessor>> _processors;
    };
}