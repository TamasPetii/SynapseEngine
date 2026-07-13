#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API SkySpherePass : public GraphicsPass {
    public:
        SkySpherePass();

        std::string GetName() const override { return "SkySpherePass"; }
        std::string GetGroup() const override { return PassGroupNames::PostProcessPasses; }
        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}