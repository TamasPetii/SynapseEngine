#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API SsaoInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "SsaoInitPass"; }
        std::string GetGroup() const override { return PassGroupNames::SsaoPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}