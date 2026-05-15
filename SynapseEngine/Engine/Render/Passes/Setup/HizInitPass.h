#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransitionPass.h"

namespace Syn {
    class SYN_API HizInitPass : public TransitionPass {
    public:
        std::string GetName() const override { return "HizInitPass"; }
        std::string GetGroup() const override { return PassGroupNames::InitSetupPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}