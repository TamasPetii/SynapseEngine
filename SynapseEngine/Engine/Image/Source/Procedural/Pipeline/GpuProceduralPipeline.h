#pragma once
#include "Engine/SynApi.h"
#include "IGpuProceduralPass.h"
#include <vector>
#include <memory>

namespace Syn {
    class SYN_API GpuProceduralPipeline {
    public:
        GpuProceduralPipeline() = default;
        GpuProceduralPipeline(const GpuProceduralPipeline&) = delete;
        GpuProceduralPipeline& operator=(const GpuProceduralPipeline&) = delete;

        void AddPass(std::unique_ptr<IGpuProceduralPass> pass);
        void InitializeAll();
        void Execute(const GpuProceduralContext& context);

    private:
        std::vector<std::unique_ptr<IGpuProceduralPass>> _passes;
    };
}