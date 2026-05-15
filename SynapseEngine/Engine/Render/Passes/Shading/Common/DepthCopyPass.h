#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"

namespace Syn {
    class SYN_API DepthCopyPass : public TransferPass {
    public:
        DepthCopyPass() = default;
        std::string GetName() const override { return "DepthCopyPass"; }
        std::string GetGroup() const override { return PassGroupNames::ShadingSetupPasses; }

        void Initialize() override {}
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void Transfer(const RenderContext& context) override;
    };
}