#pragma once
#include "Engine/Logger/LogMessage.h"
#include <vector>
#include <string>

namespace Syn {
    struct LoggerFilters {
        bool showInfo = true;
        bool showWarning = true;
        bool showError = true;
        bool showCritical = true;
        std::string searchQuery = "";
    };

    struct LoggerState {
        LoggerFilters filters;
        std::vector<LogMessage> filteredLogs;
        bool autoScroll = true;
    };
}