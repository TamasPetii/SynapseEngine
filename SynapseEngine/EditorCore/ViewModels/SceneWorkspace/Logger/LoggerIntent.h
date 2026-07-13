#pragma once
#include "Engine/Logger/LogMessage.h"
#include <variant>
#include <string>

namespace Syn {
    struct LoggerToggleLevelIntent {
        LogLevel level;
        bool isVisible;
    };

    struct LoggerSetSearchQueryIntent {
        std::string query;
    };

    struct LoggerSetAutoScrollIntent {
        bool autoScroll;
    };

    struct LoggerClearIntent {};

    using LoggerIntent = std::variant<
        LoggerToggleLevelIntent,
        LoggerSetSearchQueryIntent,
        LoggerSetAutoScrollIntent,
        LoggerClearIntent
    >;
}