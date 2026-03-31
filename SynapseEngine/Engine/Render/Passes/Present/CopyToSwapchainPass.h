#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"

namespace Syn {
    class SYN_API CopyToSwapchainPass : public TransferPass {
    public:
        std::string GetName() const override { return "CopyToSwapchainPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void Transfer(const RenderContext& context) override;
    };
}