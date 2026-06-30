#pragma once
#include <glm/glm.hpp>

namespace Syn {
    struct CapsuleColliderState {
        bool hasComponent = false;

        float radius;
        float halfHeight;
        glm::vec3 localOffset;
    };
}