#include "CpuModelProcessorPipeline.h"

namespace Syn
{
    void CpuModelProcessorPipeline::AddProcessor(std::unique_ptr<ICpuModelProcessor> processor)
    {
        _processors.push_back(std::move(processor));
    }

    void CpuModelProcessorPipeline::Run(CpuModelData& cpuData)
    {
        for (auto& processor : _processors)
        {
            processor->Process(cpuData);
        }
    }
}