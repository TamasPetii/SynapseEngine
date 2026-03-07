#pragma once
#include "Engine/SynApi.h"
#include "BaseRenderPass.h"

namespace Syn {
    class SYN_API TestMeshPass : public BaseRenderPass {
    public:
        std::string GetName() const override { return "TestMeshPass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Draw(const RenderContext& context) override;
    };
}