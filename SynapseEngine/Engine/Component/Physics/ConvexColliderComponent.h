#pragma once
#include "Engine/Component/Core/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API ConvexColliderComponent : public Component
    {
        uint32_t targetLodLevel = 0;
        glm::vec3 localOffset = glm::vec3(0.0f);
    };

    struct SYN_API ConvexColliderComponentGPU
    {
        ConvexColliderComponentGPU(const ConvexColliderComponent& component)
            : localOffset(component.localOffset), targetLodLevel(component.targetLodLevel), padding{ 0, 0 } {}

        glm::vec3 localOffset;
        uint32_t targetLodLevel;
        uint32_t padding[2];
    };
}