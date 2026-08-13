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