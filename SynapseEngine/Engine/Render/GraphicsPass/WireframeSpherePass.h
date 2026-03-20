#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/GraphicsPass/GraphicsPass.h"

namespace Syn {
    class SYN_API WireframeSpherePass : public GraphicsPass {
    public:
        std::string GetName() const override { return "WireframeSpherePass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}