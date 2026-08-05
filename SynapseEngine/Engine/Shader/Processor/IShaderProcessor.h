#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Cooked/CookedShader.h"

namespace Syn {
    class SYN_API IShaderProcessor {
    public:
        virtual ~IShaderProcessor() = default;
        virtual void Process(CookedShader& cookedShader) = 0;
    };
}