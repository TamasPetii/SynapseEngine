#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/IRenderPass.h"

namespace Syn {
    class SYN_API PointLightShadowCullingMemoryBarrierPass : public IRenderPass {
    public:
        std::string GetName() const override { return "PointLightShadowCullingMemoryBarrierPass"; }
        std::string GetGroup() const override { return PassGroupNames::PointLightCullingPasses; }

        void Execute(const RenderContext& context) override;
    };
}