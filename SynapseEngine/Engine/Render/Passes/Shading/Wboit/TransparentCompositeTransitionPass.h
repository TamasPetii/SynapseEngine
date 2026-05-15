#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API TransparentCompositeTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "TransparentCompositeTransitionPass"; }
        std::string GetGroup() const override { return PassGroupNames::WboitPasses; }
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
    };
}