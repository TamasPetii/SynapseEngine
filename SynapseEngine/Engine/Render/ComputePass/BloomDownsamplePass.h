#pragma once
#include "Engine/SynApi.h"
#include "ComputePass.h"

namespace Syn {
    class SYN_API BloomDownsamplePass : public ComputePass {
    public:
        std::string GetName() const override { return "BloomDownsamplePass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    };
}