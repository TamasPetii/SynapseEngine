#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/GraphicsPass/GraphicsPass.h"
#include "Engine/Material/MaterialRenderType.h"

namespace Syn {
    class SYN_API TraditionalTransparentPickingPass : public GraphicsPass {
    public:
        TraditionalTransparentPickingPass(MaterialRenderType renderType);

        std::string GetName() const override { return _passName; }
        void Initialize() override;
    protected:
        void BindDescriptors(const RenderContext& context) override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    private:
        MaterialRenderType _renderType;
        std::string _passName;
    };
}