#include "DefaultGpuProfiler.h"

namespace Syn {

    DefaultGpuProfiler::DefaultGpuProfiler(uint32_t framesInFlight, float timestampPeriod)
        : _framesInFlight(framesInFlight),
        _timestampPeriod(timestampPeriod)
    {
        _pools.resize(framesInFlight);
        _activeMeasurements.resize(framesInFlight);
        _resolvedTimings.resize(framesInFlight);
        _queryCounters.resize(framesInFlight, 0);

        for (uint32_t i = 0; i < framesInFlight; ++i) {
            _pools[i] = std::make_unique<Vk::TimestampQueryPool>(256);
        }
    }

    void DefaultGpuProfiler::BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) {
        _queryCounters[frameIndex] = 0;
        _activeMeasurements[frameIndex].clear();
        _pools[frameIndex]->Reset(cmd, 0, 256);
    }

    uint32_t DefaultGpuProfiler::StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& name) {
        uint32_t startId = _queryCounters[frameIndex]++;

        _pools[frameIndex]->WriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, startId);
        _activeMeasurements[frameIndex].push_back({ name, startId, 0 });
        return static_cast<uint32_t>(_activeMeasurements[frameIndex].size() - 1);
    }

    void DefaultGpuProfiler::EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t measurementIndex) {
        uint32_t endId = _queryCounters[frameIndex]++;

        _pools[frameIndex]->WriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, endId);
        _activeMeasurements[frameIndex][measurementIndex].endQueryId = endId;
    }

    void DefaultGpuProfiler::ResolveFrame(uint32_t frameIndex) {
        auto& measurements = _activeMeasurements[frameIndex];
        if (measurements.empty()) return;

        uint32_t totalQueries = _queryCounters[frameIndex];
        std::vector<uint64_t> results(totalQueries);

        if (_pools[frameIndex]->GetResults(0, totalQueries, results, false)) {
            _resolvedTimings[frameIndex].clear();

            for (const auto& m : measurements) {
                uint64_t startTick = results[m.startQueryId];
                uint64_t endTick = results[m.endQueryId];

                float ms = static_cast<float>(endTick - startTick) * _timestampPeriod / 1000000.0f;
                _resolvedTimings[frameIndex][m.passName] = ms;
            }
        }
    }

    const std::unordered_map<std::string, float>& DefaultGpuProfiler::GetTimings(uint32_t frameIndex) const {
        return _resolvedTimings[frameIndex];
    }

}