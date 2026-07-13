#pragma once
#include <glm/glm.hpp>

namespace Syn {
    struct DirectionLightState {
        bool hasComponent;

        glm::vec3 color;
        float strength;
        bool useShadow;

        float shadowFarPlane;
        glm::vec4 cascadeSplits;
    };
}