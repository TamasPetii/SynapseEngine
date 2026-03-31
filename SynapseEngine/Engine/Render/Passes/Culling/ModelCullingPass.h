#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API ModelCullingPass : public ComputePass {
    public:
        std::string GetName() const override { return "ModelCullingPass"; }
        void Initialize() override;
    protected:
        void PushConstants(const RenderContext& context) override;
		void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _totalModelsToTest = 0;
    };
}