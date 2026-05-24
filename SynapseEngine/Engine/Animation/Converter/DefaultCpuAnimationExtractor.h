#pragma once
#include "ICpuAnimationExtractor.h"

namespace Syn
{
    class SYN_API DefaultCpuAnimationExtractor : public ICpuAnimationExtractor
    {
    public:
        DefaultCpuAnimationExtractor() = default;
        ~DefaultCpuAnimationExtractor() override = default;

        void Extract(CookedAnimation& cookedData, GpuBatchedAnimation& gpuData, CpuAnimationData& outCpuData) const override;
    };
}