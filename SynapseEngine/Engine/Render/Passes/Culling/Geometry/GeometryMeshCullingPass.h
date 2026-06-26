#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API GeometryMeshCullingPass : public ComputePass {
    public:
        std::string GetName() const override { return "GeometryMeshCullingPass"; }
        std::string GetGroup() const override { return PassGroupNames::GeometryCullingPasses; }

        void Initialize() override;
    protected:
		bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
		void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        bool _shouldDispatch = false;
    };
}