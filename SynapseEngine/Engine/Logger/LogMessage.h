#pragma once
#include "Engine/SynApi.h"

#include <string>
#include <string_view>
#include <chrono>

namespace Syn
{
    enum class LogLevel {
        Info,
        Warning,
        Error,
        Critical
    };

    struct SYN_API LogMessage {
        LogLevel level;
        std::string message;
        std::string file;
        int line;
        std::chrono::system_clock::time_point time;

        std::string ToString() const;
    };
}

