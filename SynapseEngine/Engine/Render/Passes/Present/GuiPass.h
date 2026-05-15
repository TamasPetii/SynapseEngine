#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API GuiPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "GuiPass"; }
        std::string GetGroup() const override { return PassGroupNames::PresentPasses; }

        void Initialize() override;
        void Execute(const RenderContext& context) override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}