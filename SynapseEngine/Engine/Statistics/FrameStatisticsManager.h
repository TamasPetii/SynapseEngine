#pragma once
#include "Engine/SynApi.h"
#include "Engine/Statistics/IRenderStatCollector.h"
#include <vector>

namespace Syn {

    struct SYN_API CpuRenderStats {
        uint32_t totalModels = 0;
        uint32_t totalDrawDescriptors = 0;
        uint32_t traditionalDrawDescriptors = 0;
        uint32_t meshletDrawDescriptors = 0;
        uint32_t totalAllocatedInstances = 0;
        uint32_t totalMaxMeshlets = 0;
    };

    class SYN_API FrameStatisticsManager {
    public:
        FrameStatisticsManager(uint32_t framesInFlight);

        void UpdateCpuStats(const CpuRenderStats& newStats);
        void ResolveFrame(uint32_t frameIndex, IRenderStatCollector* gpuCollector);

        const CpuRenderStats& GetCpuStats() const;
        const std::vector<RenderPassStats>& GetGpuStats(uint32_t frameIndex) const;
    private:
        CpuRenderStats _cpuStats;
        std::vector<std::vector<RenderPassStats>> _gpuStatsPerFrame;
    };
}