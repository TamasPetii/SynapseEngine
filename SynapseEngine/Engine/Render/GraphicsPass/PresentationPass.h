#pragma once
#include "Engine/SynApi.h"
#include "BaseRenderPass.h"

namespace Syn {
    class SYN_API PresentationPass : public BaseRenderPass {
    public:
        void Initialize() override;
        std::string GetName() const override { return "PresentationPass"; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
    };
}