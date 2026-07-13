#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn
{
    struct SetSphereColliderRadiusIntent {
        float radius;
        bool isDragging;
    };

    struct SetSphereColliderLocalOffsetIntent {
        glm::vec3 localOffset;
        bool isDragging;
    };

    using SphereColliderIntent = std::variant<
        SetSphereColliderRadiusIntent,
        SetSphereColliderLocalOffsetIntent>;
}