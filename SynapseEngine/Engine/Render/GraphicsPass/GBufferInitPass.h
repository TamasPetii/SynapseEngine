#pragma once
#include "Engine/SynApi.h"
#include "GraphicsPass.h"

namespace Syn {
    class SYN_API GBufferInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "GBufferInitPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}