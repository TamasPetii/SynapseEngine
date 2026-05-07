#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API TransparentInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "TransparentInitPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}