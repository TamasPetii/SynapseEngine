#pragma once
#include <glm/glm.hpp>
#include <variant>
#include <cstdint>

namespace Syn
{
    struct SetConvexColliderTargetLodLevelIntent {
        uint32_t targetLodLevel;
    };

    struct SetConvexColliderLocalOffsetIntent {
        glm::vec3 localOffset;
        bool isDragging;
    };

    using ConvexColliderIntent = std::variant<
        SetConvexColliderTargetLodLevelIntent,
        SetConvexColliderLocalOffsetIntent>;
}