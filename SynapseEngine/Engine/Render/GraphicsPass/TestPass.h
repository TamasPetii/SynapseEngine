#pragma once
#include "Engine/SynApi.h"
#include "BaseRenderPass.h"

namespace Syn {
    class SYN_API TestPass : public BaseRenderPass {
    public:
        std::string GetName() const override { return "TestPass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}