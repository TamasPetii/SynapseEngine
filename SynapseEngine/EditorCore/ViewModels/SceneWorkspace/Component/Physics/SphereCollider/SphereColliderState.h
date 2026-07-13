#pragma once
#include <glm/glm.hpp>

namespace Syn {
    struct SphereColliderState {
        bool hasComponent = false;

        float radius;
        glm::vec3 localOffset;
    };
}