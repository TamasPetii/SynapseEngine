#pragma once
#include <glm/glm.hpp>

namespace Syn {
    struct PointLightState {
        bool hasComponent = false;
        
        glm::vec3 color;
        float strength;
        bool useShadow;

        float radius;
        float weakenDistance;
        
        float shadowNearPlane;
        float shadowFarPlane;
    };
}