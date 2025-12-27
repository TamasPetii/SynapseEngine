#include "LogMessage.h"
#include "LogUtils.h"

#include <filesystem>
#include <format>

namespace Syn
{
    std::string LogMessage::ToString() const {
        auto filenameStr = std::filesystem::path(file).filename().string();

        return std::format("[{}] [{}] [{}:{}] {}",
            Syn::LogUtils::FormatTime(time),
            Syn::LogUtils::LevelToString(level),
            filenameStr,
            line,
            message
        );
    }
}