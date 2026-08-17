#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API SwapchainPresentPass : public TransitionPass {
    public:
        std::string GetName() const override { return "SwapchainPresentPass"; }
        std::string GetGroup() const override { return PassGroupNames::PresentPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}