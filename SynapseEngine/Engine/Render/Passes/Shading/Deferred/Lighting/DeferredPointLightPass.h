#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API DeferredPointLightPass : public GraphicsPass {
    public:
        DeferredPointLightPass() = default;

        std::string GetName() const override { return "DeferredPointLightPass"; }
        std::string GetGroup() const override { return PassGroupNames::DeferredLightingPasses; }
        void Initialize() override;
        bool ShouldCollectStatistics() const override { return true; }
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void BindDescriptors(const RenderContext& context) override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}