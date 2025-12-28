#pragma once
#include <format>
#include <source_location>

namespace Syn
{
    template<typename... Args>
    struct LogFormat {
        std::format_string<Args...> fmt;
        std::source_location loc;

        template<typename T>
        consteval LogFormat(const T& s, std::source_location loc = std::source_location::current())
            : fmt(s), loc(loc) {
        }
    };
}