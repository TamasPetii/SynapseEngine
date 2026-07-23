#pragma once
#include <variant>
#include <string>
#include "StatisticsState.h"

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