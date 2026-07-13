#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API MortonChunkAabbWireframePass : public GraphicsPass {
    public:
        std::string GetName() const override { return "MortonChunkAabbWireframePass"; }
        std::string GetGroup() const override { return PassGroupNames::WireframePasses; }
        void Initialize() override;
        bool ShouldCollectStatistics() const override { return true; }
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}