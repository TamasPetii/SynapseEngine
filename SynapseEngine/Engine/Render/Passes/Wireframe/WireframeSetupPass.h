#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API WireframeSetupPass : public ComputePass {
    public:
        std::string GetName() const override { return "WireframeSetupPass"; }
        void Initialize() override;
    protected:
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        bool _shouldDispatch = false;
    };
}