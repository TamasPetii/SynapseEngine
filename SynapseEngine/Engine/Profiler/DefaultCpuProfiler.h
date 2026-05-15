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
        std::string groupName;
        std::string entryName; 
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
        void EndMeasurement(uint32_t frameIndex, uint32_t measurementIndex) override;
        uint32_t StartMeasurement(uint32_t frameIndex, const std::string& groupName, const std::string& name) override;

        void ResolveFrame(uint32_t frameIndex) override;
        const std::vector<GroupTiming>& GetTimings(uint32_t frameIndex) const override;

    private:
        uint32_t _framesInFlight;

        std::vector<std::vector<GroupTiming>> _resolvedTimings;
        std::vector<std::vector<CpuProfilerMeasurement>> _activeMeasurements;

        mutable std::mutex _mutex;
    };

}