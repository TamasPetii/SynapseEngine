#pragma once
#include "ICpuModelExtractor.h"

namespace Syn
{
    class SYN_API DefaultCpuModelExtractor : public ICpuModelExtractor
    {
    public:
        DefaultCpuModelExtractor() = default;
        ~DefaultCpuModelExtractor() override = default;

        void Extract(GpuBatchedModel& gpuData, CpuModelData& outCpuData) const override;
    };
}