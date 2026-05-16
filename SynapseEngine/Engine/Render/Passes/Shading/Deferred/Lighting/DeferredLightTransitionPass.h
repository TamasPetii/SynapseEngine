#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API DeferredLightTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "DeferredLightTransitionPass"; }
        std::string GetGroup() const override { return PassGroupNames::DeferredLightingPasses; }
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
    };
}