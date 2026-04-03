#pragma once
#include "BaseComponent/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API CapsuleColliderComponent : public Component
    {
        float radius = 0.5f;
        float halfHeight = 1.0f;
        glm::vec3 localOffset = glm::vec3(0.0f);
    };
}