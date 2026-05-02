#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn
{
    class SYN_API ClusterPrefixSumPass : public ComputePass {
    public:
        std::string GetName() const override { return "ClusterPrefixSumPass"; }
        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}