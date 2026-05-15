#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API BillboardTransitionPass : public TransitionPass {
    public:
        std::string GetName() const override { return "BillboardTransitionPass"; }
        std::string GetGroup() const override { return PassGroupNames::BillboardPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}