#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API TransparentCompositeTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "TransparentCompositeTransitionPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}