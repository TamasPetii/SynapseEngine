#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API PointLightShadowTransparentTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "PointLightShadowTransparentTransitionPass"; }
        std::string GetGroup() const override { return PassGroupNames::PointLightShadowPasses; }

        void Initialize() override {}
        bool ShouldCollectStatistics() const override { return false; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}