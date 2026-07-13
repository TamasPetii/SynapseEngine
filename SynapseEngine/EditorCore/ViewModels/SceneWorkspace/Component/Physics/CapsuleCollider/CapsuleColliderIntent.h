#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn
{
    struct SetCapsuleColliderRadiusIntent {
        float radius;
        bool isDragging;
    };

    struct SetCapsuleColliderHalfHeightIntent {
        float halfHeight;
        bool isDragging;
    };

    struct SetCapsuleColliderLocalOffsetIntent {
        glm::vec3 localOffset;
        bool isDragging;
    };

    using CapsuleColliderIntent = std::variant<
        SetCapsuleColliderRadiusIntent,
        SetCapsuleColliderHalfHeightIntent,
        SetCapsuleColliderLocalOffsetIntent>;
}