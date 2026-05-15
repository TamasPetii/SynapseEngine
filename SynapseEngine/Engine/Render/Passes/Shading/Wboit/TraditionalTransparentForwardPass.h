#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"
#include "Engine/Material/MaterialRenderType.h"

namespace Syn {
    class SYN_API TraditionalTransparentForwardPass : public GraphicsPass {
    public:
        TraditionalTransparentForwardPass(MaterialRenderType renderType);

        std::string GetName() const override { return _passName; }
        std::string GetGroup() const override { return PassGroupNames::WboitPasses; }
        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void BindDescriptors(const RenderContext& context) override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    private:
        MaterialRenderType _renderType;
        std::string _passName;
    };
}