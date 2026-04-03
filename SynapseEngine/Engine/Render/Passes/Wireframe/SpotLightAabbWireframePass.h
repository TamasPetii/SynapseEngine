#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API SpotLightAabbWireframePass : public GraphicsPass {
    public:
        std::string GetName() const override { return "SpotLightAabbWireframePass"; }
        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}