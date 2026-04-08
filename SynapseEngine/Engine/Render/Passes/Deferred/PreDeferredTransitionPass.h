#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API PreDeferredTransitionPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "PreDeferredTransitionPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}