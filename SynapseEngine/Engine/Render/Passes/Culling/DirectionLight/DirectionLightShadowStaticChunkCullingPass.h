#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API DirectionLightShadowStaticChunkCullingPass : public ComputePass {
    public:
        std::string GetName() const override { return "DirectionLightShadowStaticChunkCullingPass"; }
        std::string GetGroup() const override { return PassGroupNames::DirectionLightShadowCullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _activeChunkCount = 0;
        uint32_t _activeLights = 0;
    };
}