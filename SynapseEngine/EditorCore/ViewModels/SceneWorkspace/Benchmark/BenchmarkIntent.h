#pragma once
#include <variant>
#include "BenchmarkState.h"

namespace Syn {
    struct BenchmarkSwitchTabIntent {
        ProfilerTab tab;
    };

    struct BenchmarkSetThresholdsIntent {
        float warning;
        float critical; 
    };

    struct BenchmarkTogglePhaseFilterIntent {
        std::string phase;
        bool isVisible;
    };

    struct BenchmarkSetSearchQueryIntent {
        std::string query;
    };

    using BenchmarkIntent = std::variant<
        BenchmarkSwitchTabIntent,
        BenchmarkSetThresholdsIntent,
        BenchmarkTogglePhaseFilterIntent,
        BenchmarkSetSearchQueryIntent
    >;
}