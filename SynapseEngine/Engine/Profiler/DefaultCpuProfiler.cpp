#include "DefaultCpuProfiler.h"

namespace Syn {

    DefaultCpuProfiler::DefaultCpuProfiler(uint32_t framesInFlight)
        : _framesInFlight(framesInFlight)
    {
        _activeMeasurements.resize(framesInFlight);
        _resolvedTimings.resize(framesInFlight);
    }

    void DefaultCpuProfiler::BeginFrame(uint32_t frameIndex) {
        std::lock_guard<std::mutex> lock(_mutex);
        _activeMeasurements[frameIndex].clear();
    }

    uint32_t DefaultCpuProfiler::StartMeasurement(uint32_t frameIndex, const std::string& name) {
        auto now = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(_mutex);
        _activeMeasurements[frameIndex].push_back({ name, now, now });
        return static_cast<uint32_t>(_activeMeasurements[frameIndex].size() - 1);
    }

    void DefaultCpuProfiler::EndMeasurement(uint32_t frameIndex, uint32_t measurementIndex) {
        auto now = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(_mutex);
        _activeMeasurements[frameIndex][measurementIndex].endTime = now;
    }

    void DefaultCpuProfiler::ResolveFrame(uint32_t frameIndex) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto& measurements = _activeMeasurements[frameIndex];

        _resolvedTimings[frameIndex].clear();

        if (measurements.empty()) {
            return;
        }

        for (const auto& m : measurements) {
            float ms = std::chrono::duration<float, std::milli>(m.endTime - m.startTime).count();
            _resolvedTimings[frameIndex][m.passName] += ms;
        }
    }

    const std::unordered_map<std::string, float>& DefaultCpuProfiler::GetTimings(uint32_t frameIndex) const {
        return _resolvedTimings[frameIndex];
    }

}