#pragma once
#include "Engine/SynApi.h"
#include "GraphicsPass.h"

namespace Syn {
    class SYN_API DebugInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "DebugInitPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}