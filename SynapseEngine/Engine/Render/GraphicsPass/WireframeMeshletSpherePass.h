#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/GraphicsPass/GraphicsPass.h"

namespace Syn
{
    class SYN_API WireframeMeshletSpherePass : public GraphicsPass
    {
    public:
        std::string GetName() const override { return "WireframeMeshletSpherePass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}