#include "LogUtils.h"
#include <format>

namespace Syn
{
    std::string LogUtils::FormatTime(std::chrono::system_clock::time_point time) {
        auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), time };
        return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
    }

    std::string LogUtils::GetCurrentTimeForFileName() {
        auto now = std::chrono::system_clock::now();
        auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), now };
        return std::format("{:%Y-%m-%d_%H-%M-%S}", localTime);
    }

    std::string_view LogUtils::LevelToString(LogLevel level) {
        switch (level) {
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARN";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT";
        }
        return "UNK";
    }
}