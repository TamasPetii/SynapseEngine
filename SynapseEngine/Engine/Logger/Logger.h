#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Logger/LogMessage.h"
#include "Engine/Logger/ISink.h"


#include <string>
#include <string_view>
#include <format>
#include <source_location>
#include <chrono>
#include <vector>
#include <memory>

namespace Syn 
{
    class SYN_API Logger {
    public:
        static Logger& Get();

        void AddSink(std::shared_ptr<ISink> sink);
        void Dispatch(LogLevel level, std::string_view msg, const char* file, int line);
    private:
        Logger() = default;
        std::vector<std::shared_ptr<ISink>> _sinks;
    };

    template<typename... Args>
    void Info(std::format_string<Args...> fmt, Args&&... args,
        std::source_location loc = std::source_location::current())
    {
        if constexpr (Syn::EnableLogging) {
            std::string msg = std::format(fmt, std::forward<Args>(args)...);
            Logger::Get().Dispatch(LogLevel::Info, msg, loc.file_name(), static_cast<int>(loc.line()));
        }
    }

    template<typename... Args>
    void Warning(std::format_string<Args...> fmt, Args&&... args,
        std::source_location loc = std::source_location::current())
    {
        if constexpr (Syn::EnableLogging) {
            std::string msg = std::format(fmt, std::forward<Args>(args)...);
            Logger::Get().Dispatch(LogLevel::Warning, msg, loc.file_name(), static_cast<int>(loc.line()));
        }
    }

    template<typename... Args>
    void Error(std::format_string<Args...> fmt, Args&&... args,
        std::source_location loc = std::source_location::current())
    {
        if constexpr (Syn::EnableLogging) {
            std::string msg = std::format(fmt, std::forward<Args>(args)...);
            Logger::Get().Dispatch(LogLevel::Error, msg, loc.file_name(), static_cast<int>(loc.line()));
        }
    }
}