#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API SpotLightShadowInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "SpotLightShadowInitPass"; }
        std::string GetGroup() const override { return PassGroupNames::ShadowPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}