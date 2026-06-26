#pragma once
#include <glm/glm.hpp>
#include <cstdint>

namespace Syn {
    struct TransformState 
    {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };
}