#pragma once
#include "BaseComponent/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API BoxColliderComponent : public Component
    {
        glm::vec3 halfExtents = glm::vec3(0.5f);
        glm::vec3 localOffset = glm::vec3(0.0f);
    };
}