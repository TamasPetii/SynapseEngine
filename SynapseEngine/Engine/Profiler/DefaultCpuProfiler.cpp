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

    uint32_t DefaultCpuProfiler::StartMeasurement(uint32_t frameIndex, const std::string& groupName, const std::string& name) {
        auto now = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(_mutex);
        _activeMeasurements[frameIndex].push_back({ groupName, name, now, now });
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
        auto& timings = _resolvedTimings[frameIndex];

        timings.clear();

        if (measurements.empty()) {
            return;
        }

        GroupTiming* currentGroup = nullptr;

        for (const auto& m : measurements) {
            float ms = std::chrono::duration<float, std::milli>(m.endTime - m.startTime).count();

            if (currentGroup == nullptr || currentGroup->name != m.groupName) {
                auto it = std::find_if(timings.begin(), timings.end(),
                    [&](const GroupTiming& g) { return g.name == m.groupName; });

                if (it != timings.end()) {
                    currentGroup = &(*it);
                }
                else {
                    timings.push_back({ m.groupName, 0.0f, {} });
                    currentGroup = &timings.back();
                }
            }

            auto entryIt = std::find_if(currentGroup->entries.begin(), currentGroup->entries.end(),
                [&](const ProfilerEntry& e) { return e.name == m.entryName; });

            if (entryIt != currentGroup->entries.end()) {
                entryIt->timeMs += ms;
            }
            else {
                currentGroup->entries.push_back({ m.entryName, ms });
            }

            currentGroup->totalTimeMs += ms;
        }
    }

    const std::vector<GroupTiming>& DefaultCpuProfiler::GetTimings(uint32_t frameIndex) const {
        return _resolvedTimings[frameIndex];
    }
}