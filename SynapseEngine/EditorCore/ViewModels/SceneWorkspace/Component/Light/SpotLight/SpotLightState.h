#pragma once
#include <glm/glm.hpp>

namespace Syn {
    struct SpotLightState {
        bool hasComponent = false;
        glm::vec3 color;
        float strength;
        bool useShadow;
        float range;
        float weakenDistance;
        float innerAngle;
        float outerAngle;
        float shadowNearPlane;
        float shadowFarPlane;
    };
}