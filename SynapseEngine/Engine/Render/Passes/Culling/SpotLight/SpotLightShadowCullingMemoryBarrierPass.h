#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/IRenderPass.h"

namespace Syn {
    class SYN_API SpotLightShadowCullingMemoryBarrierPass : public IRenderPass {
    public:
        std::string GetName() const override { return "SpotLightShadowCullingMemoryBarrierPass"; }
        std::string GetGroup() const override { return PassGroupNames::SpotLightCullingPasses; }

        void Execute(const RenderContext& context) override;
    };
}