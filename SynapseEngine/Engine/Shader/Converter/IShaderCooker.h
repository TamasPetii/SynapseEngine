#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Raw/RawShader.h"
#include "Engine/Shader/Data/Cooked/CookedShader.h"

namespace Syn {
    class SYN_API IShaderCooker {
    public:
        virtual ~IShaderCooker() = default;
        virtual CookedShader Cook(const RawShader& rawShader) = 0;
    };
}