#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn
{
    struct SetBoxColliderHalfExtentsIntent {
        glm::vec3 halfExtents;
        bool isDragging;
    };

    struct SetBoxColliderLocalOffsetIntent {
        glm::vec3 localOffset;
        bool isDragging;
    };

    using BoxColliderIntent = std::variant<
        SetBoxColliderHalfExtentsIntent,
        SetBoxColliderLocalOffsetIntent>;
}