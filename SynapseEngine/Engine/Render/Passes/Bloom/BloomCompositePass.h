#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API BloomCompositePass : public ComputePass {
    public:
        struct Config {
            float exposure = 1.0f;
            float bloomStrength = 1.0f;
        };
    public:
        std::string GetName() const override { return "BloomCompositePass"; }
        void Initialize() override;
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        Config _config;
    };
}