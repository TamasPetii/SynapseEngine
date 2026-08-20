#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API SpotLightShadowTransparentTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "SpotLightShadowTransparentTransitionPass"; }
        std::string GetGroup() const override { return PassGroupNames::SpotLightShadowPasses; }

        void Initialize() override {}
        bool ShouldCollectStatistics() const override { return false; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}