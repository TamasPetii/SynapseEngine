#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <unordered_map>
#include <format>

namespace Syn {

    class SYN_API IProfiler {
    public:
        virtual ~IProfiler() = default;

        virtual void ResolveFrame(uint32_t frameIndex) = 0;
        virtual const std::unordered_map<std::string, float>& GetTimings(uint32_t frameIndex) const = 0;

        virtual std::string GenerateReport(uint32_t frameIndex, const std::string& title) const {
            const auto& timings = GetTimings(frameIndex);
            if (timings.empty()) {
                return "";
            }

            std::string report = std::format("{} Timings:\n", title);
            float total = 0.0f;

            for (const auto& [name, ms] : timings) {
                report += std::format("    - {:<30} : {:>8.3f} ms\n", name, ms);
                total += ms;
            }

            report += "    ------------------------------------------------\n";
            report += std::format("    = {:<30} : {:>8.3f} ms\n", "TOTAL " + title + " TIME", total);

            return report;
        }
    };

}