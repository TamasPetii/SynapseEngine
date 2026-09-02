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

#include "IProfiler.h"
#include <format>

namespace Syn
{
    std::string IProfiler::GenerateReport(uint32_t frameIndex, const std::string& title) const {
        const auto& groups = GetTimings(frameIndex);
        if (groups.empty()) return "";

        const std::string cReset = "\033[0m";
        const std::string cBold = "\033[1m";
        const std::string cCyan = "\033[38;5;45m";
        const std::string cBlue = "\033[38;5;33m";
        const std::string cGreen = "\033[38;5;40m";
        const std::string cYellow = "\033[38;5;220m";
        const std::string cRed = "\033[38;5;196m";
        const std::string cGray = "\033[38;5;244m";

        float totalTime = 0.0f;
        for (const auto& group : groups) {
            totalTime += group.totalTimeMs;
        }

        std::string report = "\n";
        report += std::format("{}{}+----------------------------------------------------------------------------------+------------+----------+{}\n", cBold, cCyan, cReset);

        report += std::format("{}| {}{:<80}{} {}| {}{:>10}{} {}| {}{:>8}{} {}|{}\n",
            cCyan,
            cBold, std::string(title + " REPORT"), cReset,
            cCyan,
            cBold, std::string("TIME (ms)"), cReset,
            cCyan,
            cBold, std::string("% OF TOT"), cReset,
            cCyan,
            cReset);

        report += std::format("{}{}+----------------------------------------------------------------------------------+------------+----------+{}\n", cBold, cCyan, cReset);

        for (size_t i = 0; i < groups.size(); ++i) {
            const auto& group = groups[i];
            float groupPct = (totalTime > 0.0f) ? ((group.totalTimeMs / totalTime) * 100.0f) : 0.0f;

            report += std::format("{}| {}{:<80}{} {}| {}{:>10.3f}{} {}| {}{:>7.1f}%{} {}|{}\n",
                cCyan,
                cBold, group.name, cReset,
                cCyan,
                cYellow, group.totalTimeMs, cReset,
                cCyan,
                cBlue, groupPct, cReset,
                cCyan,
                cReset);

            for (size_t j = 0; j < group.entries.size(); ++j) {
                const auto& entry = group.entries[j];
                bool isLast = (j == group.entries.size() - 1);
                std::string treeBranch = isLast ? "\\- " : "|- ";

                std::string timeColor = cGreen;
                if (entry.timeMs < 0.01f) timeColor = cGray;
                else if (entry.timeMs > 1.0f) timeColor = cRed;

                report += std::format("{}|   {}{}{:<75}{} {}| {}{:>10.3f}{} {}| {}{:>8}{} {}|{}\n",
                    cCyan,
                    cGray, treeBranch, entry.name, cReset,
                    cCyan,
                    timeColor, entry.timeMs, cReset,
                    cCyan,
                    cGray, std::string("-"), cReset,
                    cCyan,
                    cReset);
            }

            if (i < groups.size() - 1) {
                report += std::format("{}|                                                                                  |            |          |{}\n", cCyan, cReset);
            }
        }

        report += std::format("{}{}+----------------------------------------------------------------------------------+------------+----------+{}\n", cBold, cCyan, cReset);

        report += std::format("{}| {}{:<80}{} {}| {}{:>10.3f}{} {}| {}{:>7.1f}%{} {}|{}\n",
            cCyan,
            cBold, std::string("TOTAL ") + title + std::string(" TIME"), cReset,
            cCyan,
            cBold + cYellow, totalTime, cReset,
            cCyan,
            cBold + cBlue, 100.0f, cReset,
            cCyan,
            cReset);

        report += std::format("{}{}+----------------------------------------------------------------------------------+------------+----------+{}\n", cBold, cCyan, cReset);

        return report;
    }
}