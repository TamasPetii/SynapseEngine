#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API GuiPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "GuiPass"; }
        void Initialize() override;
        void Execute(const RenderContext& context) override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}