#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"
#include <vector>

namespace Syn
{
    class SYN_API AnimationPreviewPass : public GraphicsPass {
    public:
        AnimationPreviewPass() = default;

        std::string GetName() const override { return "AnimationPreviewPass"; }
        std::string GetGroup() const override { return PassGroupNames::UtilityPasses; }

        void Initialize() override;
        bool ShouldCollectStatistics() const override { return false; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    private:
        std::vector<uint32_t> _dirtyAnimations;
    };
}