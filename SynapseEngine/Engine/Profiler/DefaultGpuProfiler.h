// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"
#include "IGpuProfiler.h"
#include "Engine/Vk/Query/TimestampQueryPool.h"

#include <vector>
#include <memory>
#include <string>

namespace Syn {

    struct SYN_API GpuProfilerMeasurement {
        std::string groupName;
        std::string entryName;
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
        void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t measurementIndex) override;
        uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& groupName, const std::string& name) override;

        void ResolveFrame(uint32_t frameIndex) override;
        const std::vector<GroupTiming>& GetTimings(uint32_t frameIndex) const override;
    private:
        static constexpr uint32_t MAX_QUERIES_PER_FRAME = 1024;

        float _timestampPeriod = 1.0f;
        uint32_t _framesInFlight;
        std::vector<uint32_t> _queryCounters;
        std::vector<std::unique_ptr<Vk::TimestampQueryPool>> _pools;

        std::vector<std::vector<GroupTiming>> _resolvedTimings;
        std::vector<std::vector<GpuProfilerMeasurement>> _activeMeasurements;
    };

}