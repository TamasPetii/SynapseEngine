#pragma once
#include "Engine/SynApi.h"
#include "ICpuProfiler.h"

#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace Syn {

    struct SYN_API CpuProfilerMeasurement {
        std::string passName;
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point endTime;
    };

    class SYN_API DefaultCpuProfiler : public ICpuProfiler {
    public:
        explicit DefaultCpuProfiler(uint32_t framesInFlight);
        ~DefaultCpuProfiler() override = default;

        DefaultCpuProfiler(const DefaultCpuProfiler&) = delete;
        DefaultCpuProfiler& operator=(const DefaultCpuProfiler&) = delete;

        void BeginFrame(uint32_t frameIndex) override;
        uint32_t StartMeasurement(uint32_t frameIndex, const std::string& name) override;
        void EndMeasurement(uint32_t frameIndex, uint32_t measurementIndex) override;

        void ResolveFrame(uint32_t frameIndex) override;
        const std::unordered_map<std::string, float>& GetTimings(uint32_t frameIndex) const override;

    private:
        uint32_t _framesInFlight;

        std::vector<std::vector<CpuProfilerMeasurement>> _activeMeasurements;
        std::vector<std::unordered_map<std::string, float>> _resolvedTimings;

        mutable std::mutex _mutex;
    };

}