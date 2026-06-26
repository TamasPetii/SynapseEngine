#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API MortonGeneratorPass : public ComputePass {
    public:
        std::string GetName() const override { return "MortonGeneratorPass"; }
        std::string GetGroup() const override { return PassGroupNames::MortonPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _staticCount = 0;
        mutable bool _wasEnabled = false;
        mutable bool _needsRebuild = true;
        uint32_t _countdown = 0;
    };
}