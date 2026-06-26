#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/IRenderPass.h"

namespace Syn {
    class SYN_API SpotLightShadowBufferResetPass : public IRenderPass {
    public:
        std::string GetName() const override { return "SpotLightShadowBufferResetPass"; }
        std::string GetGroup() const override { return PassGroupNames::SpotLightCullingPasses; }

        void Execute(const RenderContext& context) override;
    };
}