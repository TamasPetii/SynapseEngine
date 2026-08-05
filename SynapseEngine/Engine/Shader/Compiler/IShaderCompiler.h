#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Cooked/CookedShader.h"
#include <vector>
#include <cstdint>

namespace Syn {
    class SYN_API IShaderCompiler {
    public:
        virtual ~IShaderCompiler() = default;
        virtual std::vector<uint32_t> Compile(const CookedShader& shader) = 0;
    };
}