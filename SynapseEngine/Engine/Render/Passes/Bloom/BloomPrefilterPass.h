#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"
#include <glm/glm.hpp>

namespace Syn {
    class SYN_API BloomPrefilterPass : public ComputePass {
    public:
        struct Config {
            float threshold = 1.0f;
            float knee = 0.1f;
        };
    public:
        std::string GetName() const override { return "BloomPrefilterPass"; }
        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        Config _config;
    };
}