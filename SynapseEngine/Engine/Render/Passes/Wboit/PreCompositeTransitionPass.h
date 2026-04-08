#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API PreCompositeTransitionPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "PreCompositeTransitionPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}