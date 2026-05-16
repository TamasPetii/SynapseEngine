#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API HizDownsamplePass : public ComputePass {
    public:
        std::string GetName() const override { return "HizDownsamplePass"; }
        std::string GetGroup() const override { return PassGroupNames::HizPasses; }

        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}