#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"
#include "Engine/Material/MaterialRenderType.h"

namespace Syn
{
    class SYN_API PerformanceMeshletOpaquePass : public GraphicsPass {
    public:
        PerformanceMeshletOpaquePass(MaterialRenderType renderType);

        std::string GetName() const override { return _passName; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    private:
        MaterialRenderType _renderType;
        std::string _passName;
    };
}