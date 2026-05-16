#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API SceneAabbPass : public ComputePass {
    public:
        std::string GetName() const override { return "SceneAabbPass"; }
        std::string GetGroup() const override { return PassGroupNames::MortonPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _staticCount = 0;
    };
}