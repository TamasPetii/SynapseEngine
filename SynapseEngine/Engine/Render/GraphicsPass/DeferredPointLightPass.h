#pragma once
#include "Engine/SynApi.h"
#include "GraphicsPass.h"

namespace Syn {
    class SYN_API DeferredPointLightPass : public GraphicsPass {
    public:
        DeferredPointLightPass() = default;

        std::string GetName() const override { return "DeferredPointLightPass"; }
        void Initialize() override;
    protected:
        void BindDescriptors(const RenderContext& context) override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}