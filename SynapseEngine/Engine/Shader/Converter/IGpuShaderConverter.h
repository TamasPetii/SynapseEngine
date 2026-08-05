#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Cooked/CookedShader.h"
#include "Engine/Shader/Data/Gpu/GpuShaderData.h"

namespace Syn {
    class SYN_API IGpuShaderConverter {
    public:
        virtual ~IGpuShaderConverter() = default;
        virtual GpuShaderData Convert(const CookedShader& cookedShader) = 0;
    };
}