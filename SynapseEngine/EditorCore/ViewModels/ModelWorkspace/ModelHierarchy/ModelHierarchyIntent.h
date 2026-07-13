#pragma once
#include <string>
#include <variant>
#include <cstdint>

namespace Syn {
    struct ModelHierarchySelectIntent {
        uint32_t modelId;
        int32_t descriptorIndex;
    };

    struct ModelHierarchyToggleExpandIntent {
        uint32_t modelId;
        int32_t descriptorIndex;
        bool expand;
    };

    struct ModelHierarchySetSearchIntent {
        std::string query;
    };

    struct ModelHierarchyRefreshIntent {};

    using ModelHierarchyIntent = std::variant<
        ModelHierarchySelectIntent,
        ModelHierarchyToggleExpandIntent,
        ModelHierarchySetSearchIntent,
        ModelHierarchyRefreshIntent
    >;
}