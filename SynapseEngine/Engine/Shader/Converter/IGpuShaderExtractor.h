#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Gpu/GpuShaderData.h"
#include "Engine/Shader/Data/Cpu/CpuShaderData.h"

namespace Syn {
    class SYN_API IGpuShaderExtractor {
    public:
        virtual ~IGpuShaderExtractor() = default;
        virtual void Extract(GpuShaderData& gpuData) = 0;
    };
}