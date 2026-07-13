#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API PreviewPreTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "PreviewPreTransitionPass"; }
        std::string GetGroup() const override { return PassGroupNames::UtilityPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}