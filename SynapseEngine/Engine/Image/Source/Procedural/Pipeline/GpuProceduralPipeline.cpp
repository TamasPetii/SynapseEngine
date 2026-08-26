#include "GpuProceduralPipeline.h"

namespace Syn {
    void GpuProceduralPipeline::AddPass(std::unique_ptr<IGpuProceduralPass> pass) {
        _passes.push_back(std::move(pass));
    }

    void GpuProceduralPipeline::InitializeAll() {
        for (auto& pass : _passes) {
            pass->Initialize();
        }
    }

    void GpuProceduralPipeline::Execute(const GpuProceduralContext& context) {
        for (auto& pass : _passes) {
            pass->Execute(context);
        }
    }
}