#pragma once
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API ClusterPointLightSinglePassHaradaPlus : public ComputePass {
    public:
        void Initialize() override;
        std::string GetName() const override { return "ClusterPointLightSinglePassHaradaPlus"; }
        std::string GetGroup() const override { return PassGroupNames::ForwardPlusClusterPasses; }
    protected:
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}