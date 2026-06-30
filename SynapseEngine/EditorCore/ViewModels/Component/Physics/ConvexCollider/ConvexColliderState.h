#pragma once
#include <glm/glm.hpp>
#include <cstdint>

namespace Syn {
    struct ConvexColliderState {
        bool hasComponent = false;

        uint32_t targetLodLevel;
        glm::vec3 localOffset;
    };
}