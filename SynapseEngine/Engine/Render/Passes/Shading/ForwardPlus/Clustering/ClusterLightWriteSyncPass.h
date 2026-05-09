#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn 
{
    class SYN_API ClusterLightWriteSyncPass : public ComputePass {
    public:
        std::string GetName() const override { return "ClusterLightWriteSyncPass"; }
        void Initialize() override {}
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override {}
        void BindDescriptors(const RenderContext& context) override {}
        void Dispatch(const RenderContext& context) override;
    };
}