#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API TransparentDepthTransitionPrepass : public TransitionPass {
    public:
        std::string GetName() const override { return "TransparentDepthTransitionPrepass"; }
        std::string GetGroup() const override { return PassGroupNames::ForwardPlusDepthPrePasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}