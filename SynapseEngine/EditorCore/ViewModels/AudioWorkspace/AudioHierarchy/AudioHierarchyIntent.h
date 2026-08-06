#pragma once
#include <string>
#include <variant>
#include <cstdint>

namespace Syn {
    struct AudioHierarchySelectIntent { uint32_t audioId; };
    struct AudioHierarchySetSearchIntent { std::string query; };
    struct AudioHierarchyRefreshIntent {};

    using AudioHierarchyIntent = std::variant<
        AudioHierarchySelectIntent,
        AudioHierarchySetSearchIntent,
        AudioHierarchyRefreshIntent
    >;
}