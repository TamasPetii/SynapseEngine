#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API ClusterPointLightSinglePass : public ComputePass {
    public:
        std::string GetName() const override { return "ClusterPointLightSinglePass"; }
        std::string GetGroup() const override { return PassGroupNames::ForwardPlusClusterPasses; }
        void Initialize() override;
    protected:
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}