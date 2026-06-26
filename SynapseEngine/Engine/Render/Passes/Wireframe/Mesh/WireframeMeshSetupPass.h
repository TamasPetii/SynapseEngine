#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API WireframeMeshSetupPass : public ComputePass {
    public:
        std::string GetName() const override { return "WireframeMeshSetupPass"; }
        std::string GetGroup() const override { return PassGroupNames::WireframePasses; }
        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        bool _shouldDispatch = false;
    };
}