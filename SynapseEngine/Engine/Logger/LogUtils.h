#pragma once
#include "Engine/SynApi.h"
#include "Engine/Logger/LogMessage.h"
#include <string>
#include <chrono>

namespace Syn
{
    class SYN_API LogUtils {
    public:
        static std::string FormatTime(std::chrono::system_clock::time_point time);
        static std::string GetCurrentTimeForFileName();
        static std::string_view LevelToString(LogLevel level);
    };
}