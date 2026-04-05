#pragma once
#include "Engine/SynApi.h"
#include "IGpuProfiler.h"
#include "Engine/Vk/Query/TimestampQueryPool.h"

#include <vector>
#include <memory>
#include <string>

namespace Syn {

    struct SYN_API ProfilerMeasurement {
        std::string passName;
        uint32_t startQueryId;
        uint32_t endQueryId;
    };

    class SYN_API DefaultGpuProfiler : public IGpuProfiler {
    public:
        explicit DefaultGpuProfiler(uint32_t framesInFlight, float timestampPeriod);
        ~DefaultGpuProfiler() override = default;

        DefaultGpuProfiler(const DefaultGpuProfiler&) = delete;
        DefaultGpuProfiler& operator=(const DefaultGpuProfiler&) = delete;

        void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) override;
        uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& name) override;
        void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t measurementIndex) override;

        void ResolveFrame(uint32_t frameIndex) override;
        const std::unordered_map<std::string, float>& GetTimings(uint32_t frameIndex) const override;
    private:
        float _timestampPeriod = 1.0f;
        uint32_t _framesInFlight;
        std::vector<uint32_t> _queryCounters;
        std::vector<std::unique_ptr<Vk::TimestampQueryPool>> _pools;

        std::vector<std::vector<ProfilerMeasurement>> _activeMeasurements;
        std::vector<std::unordered_map<std::string, float>> _resolvedTimings;
    };

}