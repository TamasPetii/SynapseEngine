#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API PerformanceInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "PerformanceInitPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}