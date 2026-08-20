#pragma once
#include "Engine/SynApi.h"
#include "Engine/Environment/Bake/EnvironmentComputeBaker.h"

namespace Syn 
{
    class SYN_API EquirectangularToCubeBaker : public EnvironmentComputeBaker {
    public:
        void Initialize() override;
    protected:
        void BindDescriptors(const EnvironmentBakeContext& context) override;
        void Dispatch(const EnvironmentBakeContext& context) override;
    };
}