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