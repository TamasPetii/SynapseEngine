#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API DirectionLightShadowInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "DirectionLightShadowInitPass"; }
        std::string GetGroup() const override { return PassGroupNames::DirectionLightShadowPasses; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}