#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API InfiniteGridPass : public GraphicsPass {
    public:
        InfiniteGridPass() = default;

        std::string GetName() const override { return "InfiniteGridPass"; }
        std::string GetGroup() const override { return PassGroupNames::DebugPasses; }
        void Initialize() override;
        bool ShouldCollectStatistics() const override { return true; }
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}