#pragma once
#include "Engine/SynApi.h"
#include "IShaderCooker.h"

namespace Syn {
    class SYN_API DefaultShaderCooker : public IShaderCooker {
    public:
        DefaultShaderCooker() = default;
        ~DefaultShaderCooker() override = default;

        CookedShader Cook(const RawShader& rawShader) override;
    };
}