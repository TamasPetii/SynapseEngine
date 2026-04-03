#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API BloomUpsamplePass : public ComputePass {
    public:
        struct Config {
            float filterRadius = 0.005f;
        };
    public:
        std::string GetName() const override { return "BloomUpsamplePass"; }
        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        Config _config;
    };
}