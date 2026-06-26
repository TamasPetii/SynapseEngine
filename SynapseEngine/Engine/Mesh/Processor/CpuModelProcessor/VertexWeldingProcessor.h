#pragma once
#include "Engine/SynApi.h"
#include "ICpuModelProcessor.h"

namespace Syn
{
    class SYN_API VertexWeldingProcessor : public ICpuModelProcessor
    {
    public:
        void Process(CpuModelData& cpuData) override;
    };
}