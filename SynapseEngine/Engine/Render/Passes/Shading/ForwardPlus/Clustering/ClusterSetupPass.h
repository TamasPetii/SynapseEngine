#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {
    class SYN_API ClusterSetupPass : public ComputePass {
    public:
        std::string GetName() const override { return "ClusterSetupPass"; }
        void Initialize() override;
    protected:
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}