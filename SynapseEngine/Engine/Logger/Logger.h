#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Logger/LogMessage.h"
#include "Engine/Logger/LogFormat.h" // <--- Az új header include-olása
#include "Sink/ISink.h"

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <type_traits>

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
    void Info(Syn::LogFormat<std::type_identity_t<Args>...> format, Args&&... args)
    {
        if constexpr (Syn::EnableLogging) {
            std::string msg = std::format(format.fmt, std::forward<Args>(args)...);
            Logger::Get().Dispatch(LogLevel::Info, msg, format.loc.file_name(), static_cast<int>(format.loc.line()));
        }
    }

    template<typename... Args>
    void Warning(Syn::LogFormat<std::type_identity_t<Args>...> format, Args&&... args)
    {
        if constexpr (Syn::EnableLogging) {
            std::string msg = std::format(format.fmt, std::forward<Args>(args)...);
            Logger::Get().Dispatch(LogLevel::Warning, msg, format.loc.file_name(), static_cast<int>(format.loc.line()));
        }
    }

    template<typename... Args>
    void Error(Syn::LogFormat<std::type_identity_t<Args>...> format, Args&&... args)
    {
        if constexpr (Syn::EnableLogging) {
            std::string msg = std::format(format.fmt, std::forward<Args>(args)...);
            Logger::Get().Dispatch(LogLevel::Error, msg, format.loc.file_name(), static_cast<int>(format.loc.line()));
        }
    }
}