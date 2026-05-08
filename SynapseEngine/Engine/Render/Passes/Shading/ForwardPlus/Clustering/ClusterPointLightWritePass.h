#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn
{
    class SYN_API ClusterPointLightWritePass : public ComputePass {
    public:
        std::string GetName() const override { return "ClusterPointLightWritePass"; }
        void Initialize() override;
    protected:
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}