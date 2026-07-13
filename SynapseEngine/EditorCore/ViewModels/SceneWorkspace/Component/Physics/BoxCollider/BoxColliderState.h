#pragma once
#include <glm/glm.hpp>

namespace Syn {
    struct BoxColliderState {
        bool hasComponent = false;

        glm::vec3 halfExtents;
        glm::vec3 localOffset;
    };
}