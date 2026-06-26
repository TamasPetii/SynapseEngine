#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API PointLightShadowFinalizePass : public ComputePass {
    public:
        std::string GetName() const override { return "PointLightShadowFinalizePass"; }
        std::string GetGroup() const override { return PassGroupNames::PointLightCullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}