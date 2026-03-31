#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"

namespace Syn {
    class SYN_API PresentationPass : public TransferPass {
    public:
        std::string GetName() const override { return "PresentationPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}