#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API PresentationTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "PresentationTransitionPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}