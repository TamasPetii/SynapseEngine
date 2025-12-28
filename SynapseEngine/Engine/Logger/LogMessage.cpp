#include "LogMessage.h"
#include "LogUtils.h"

#include <filesystem>
#include <format>

namespace Syn
{
    std::string LogMessage::ToString() const {
        auto filenameStr = std::filesystem::path(file).filename().string();

        return std::format("[{}] [{}] [{}:{}] {}",
            LogUtils::FormatTime(time),
            LogUtils::LevelToString(level),
            filenameStr,
            line,
            message
        );
    }
}