#pragma once
#include "Engine/SynApi.h"
#include "ComputePass.h"

namespace Syn {
    class SYN_API MeshCullingPass : public ComputePass {
    public:
        std::string GetName() const override { return "MeshCullingPass"; }
        void Initialize() override;
    protected:
        void PushConstants(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        bool _shouldDispatch = false;
    };
}