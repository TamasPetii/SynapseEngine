#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"

namespace Syn {
    class SYN_API GlobalFrameSetupPass : public TransferPass {
    public:
        std::string GetName() const override { return "GlobalFrameSetupPass"; }
    protected:
        void Transfer(const RenderContext& context) override;
    };
}