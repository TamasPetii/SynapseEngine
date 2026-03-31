#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API CompositePass : public GraphicsPass {
    public:
        std::string GetName() const override { return "CompositePass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}