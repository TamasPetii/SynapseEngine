#pragma once
#include "Engine/SynApi.h"
#include "Engine/Environment/Bake/EnvironmentComputeBaker.h"

namespace Syn 
{
    class SYN_API PrefilterBaker : public EnvironmentComputeBaker {
    public:
        void Initialize() override;
        void Bake(const EnvironmentBakeContext& context) override;
    };
}