#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn
{
    class SYN_API WireframeMeshletAabbPass : public GraphicsPass
    {
    public:
        std::string GetName() const override { return "WireframeMeshletAABBPass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}