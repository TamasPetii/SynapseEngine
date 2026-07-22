#pragma once
#include <variant>
#include <string>
#include "Engine/Statistics/FrameStatisticsManager.h"

namespace Syn {
    struct StatisticsSetSearchQueryIntent {
        std::string query;
    };

    struct StatisticsSwitchTabIntent {
        StatCategory tab;
    };

    using StatisticsIntent = std::variant<
        StatisticsSetSearchQueryIntent,
        StatisticsSwitchTabIntent
    >;
}