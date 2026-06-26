#pragma once
#include <string>
#include <variant>

namespace Syn {
    struct MaterialSelectIntent {
        uint32_t materialId;
    };

    struct MaterialSetSearchQueryIntent {
        std::string query;
    };

    struct MaterialRefreshIntent {};

    using MaterialHierarchyIntent = std::variant<
        MaterialSelectIntent,
        MaterialSetSearchQueryIntent,
        MaterialRefreshIntent
    >;
}