#pragma once
#include "Engine/SynApi.h"
#include "Engine/Logger/LogMessage.h"
#include <string>
#include <chrono>

namespace Syn::LogUtils
{

    SYN_API std::string FormatTime(std::chrono::system_clock::time_point time);

    SYN_API std::string GetCurrentTimeForFileName();

    SYN_API std::string_view LevelToString(LogLevel level);
}