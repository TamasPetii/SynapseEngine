#pragma once
#include <string>
#include <variant>
#include <cstdint>

namespace Syn {
    struct AnimationHierarchySelectIntent {
        uint32_t animationId;
        int32_t descriptorIndex;
    };

    struct AnimationHierarchyToggleExpandIntent {
        uint32_t animationId;
        int32_t descriptorIndex;
        bool expand;
    };

    struct AnimationHierarchySetSearchIntent {
        std::string query;
    };

    struct AnimationHierarchyRefreshIntent {};

    using AnimationHierarchyIntent = std::variant<
        AnimationHierarchySelectIntent,
        AnimationHierarchyToggleExpandIntent,
        AnimationHierarchySetSearchIntent,
        AnimationHierarchyRefreshIntent
    >;
}