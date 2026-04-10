#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn { 
    class SYN_API CullingCommandResetPass : public ComputePass {
    public:
        std::string GetName() const override { return "CullingCommandResetPass"; }
        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _totalCommands = 0;
    };
}