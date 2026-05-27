#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Cpu/CpuAnimationData.h"

namespace Syn
{
    class SYN_API ICpuAnimationExtractor
    {
    public:
        virtual ~ICpuAnimationExtractor() = default;
        virtual void Extract(GpuBatchedAnimation& gpuData, CpuAnimationData& outCpuData) const = 0;
    };
}