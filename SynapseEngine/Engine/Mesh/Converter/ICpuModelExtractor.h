#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    class SYN_API ICpuModelExtractor
    {
    public:
        virtual ~ICpuModelExtractor() = default;
        virtual void Extract(GpuBatchedModel& gpuData, CpuModelData& outCpuData) const = 0;
    };
}