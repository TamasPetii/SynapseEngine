#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API DeferredSpotLightPass : public GraphicsPass {
    public:
        DeferredSpotLightPass() = default;

        std::string GetName() const override { return "DeferredSpotLightPass"; }
        void Initialize() override;
    protected:
        void BindDescriptors(const RenderContext& context) override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}