#include "FrameStatisticsManager.h"

namespace Syn {

    FrameStatisticsManager::FrameStatisticsManager(uint32_t framesInFlight) {
        _gpuStatsPerFrame.resize(framesInFlight);
    }

    void FrameStatisticsManager::UpdateCpuStats(const CpuRenderStats& newStats) {
        _cpuStats = newStats;
    }

    void FrameStatisticsManager::ResolveFrame(uint32_t frameIndex, IRenderStatCollector* gpuCollector) {
        if (gpuCollector) {
            gpuCollector->ResolveFrame(frameIndex);
            _gpuStatsPerFrame[frameIndex] = gpuCollector->GetStats(frameIndex);
        }
    }

    const CpuRenderStats& FrameStatisticsManager::GetCpuStats() const {
        return _cpuStats;
    }

    const std::vector<RenderPassStats>& FrameStatisticsManager::GetGpuStats(uint32_t frameIndex) const {
        return _gpuStatsPerFrame[frameIndex];
    }
}