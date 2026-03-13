#pragma once
#include "Engine/SynApi.h"
#include "GraphicsPass.h"

namespace Syn 
{
    class SYN_API MeshletRenderPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "MeshletRenderPass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}