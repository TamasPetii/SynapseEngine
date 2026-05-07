#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API TransparentCompositePass : public GraphicsPass {
    public:
        TransparentCompositePass() = default;

        std::string GetName() const override { return "Transparent_Composite"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}