#pragma once
#include "Engine/SynApi.h"
#include "GpuProceduralContext.h"

namespace Syn
{
    class SYN_API IGpuProceduralPass {
    public:
        virtual ~IGpuProceduralPass() = default;
        virtual void Initialize() = 0;
        virtual void Execute(const GpuProceduralContext& context) = 0;
    };
}