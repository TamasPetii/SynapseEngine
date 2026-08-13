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
#include <string>
#include <unordered_map>
#include <format>
#include <vector>

namespace Syn 
{
    struct SYN_API ProfilerEntry {
        std::string name;
        float timeMs;
    };

    struct SYN_API GroupTiming {
        std::string name;
        float totalTimeMs;
        std::vector<ProfilerEntry> entries;
    };

    class SYN_API IProfiler {
    public:
        virtual ~IProfiler() = default;

        virtual void ResolveFrame(uint32_t frameIndex) = 0;
        virtual const std::vector<GroupTiming>& GetTimings(uint32_t frameIndex) const = 0;

        virtual std::string GenerateReport(uint32_t frameIndex, const std::string& title) const {
            const auto& groups = GetTimings(frameIndex);
            if (groups.empty()) return "";

            std::string report = std::format("{} Timings:\n", title);
            float totalTime = 0.0f;

            for (const auto& group : groups) {
                report += std::format("    +---[ {} ] - Total: {:.3f} ms\n", group.name, group.totalTimeMs);

                for (const auto& entry : group.entries) {
                    report += std::format("    |   {:<42} : {:>8.3f} ms\n", entry.name, entry.timeMs);
                }

                totalTime += group.totalTimeMs;
            }

            report += "    ----------------------------------------------------------------------\n";
            report += std::format("    = {:<44} : {:>8.3f} ms\n", "TOTAL " + title + " TIME", totalTime);

            return report;
        }
    };

}