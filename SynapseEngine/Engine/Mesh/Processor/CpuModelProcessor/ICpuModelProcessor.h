#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    class SYN_API ICpuModelProcessor
    {
    public:
        virtual ~ICpuModelProcessor() = default;
        virtual void Process(CpuModelData& cpuData) = 0;
    };
}