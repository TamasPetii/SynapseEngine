#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Gpu/GpuShaderData.h"
#include "Engine/Shader/Data/Cpu/CpuShaderData.h"

namespace Syn {
    class SYN_API ICpuShaderExtractor {
    public:
        virtual ~ICpuShaderExtractor() = default;
        virtual void Extract(const GpuShaderData& gpuData, CpuShaderData& outCpuData) const = 0;
    };
}