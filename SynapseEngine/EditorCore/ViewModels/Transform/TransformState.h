#pragma once
#include <glm/glm.hpp>
#include <cstdint>

namespace Syn {
    struct TransformState 
    {
        uint32_t activeEntityId = 0;
        bool hasSelection = false;

        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };
}