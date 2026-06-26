#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API PointLightShadowMortonChunkCullingPass : public ComputePass {
    public:
        std::string GetName() const override { return "PointLightShadowMortonChunkCullingPass"; }
        std::string GetGroup() const override { return PassGroupNames::PointLightCullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        bool _shouldDispatch = false;
        uint32_t _staticCount = 0;
    };
}