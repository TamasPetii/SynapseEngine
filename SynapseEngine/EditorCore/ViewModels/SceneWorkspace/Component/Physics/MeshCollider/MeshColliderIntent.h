#pragma once
#include <glm/glm.hpp>
#include <variant>
#include <cstdint>

namespace Syn
{
    struct SetMeshColliderTargetLodLevelIntent {
        uint32_t targetLodLevel;
    };

    struct SetMeshColliderLocalOffsetIntent {
        glm::vec3 localOffset;
        bool isDragging;
    };

    using MeshColliderIntent = std::variant<
        SetMeshColliderTargetLodLevelIntent,
        SetMeshColliderLocalOffsetIntent>;
}