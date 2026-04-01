#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "BaseComponent/Light.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API DirectionLightComponent : public Component, public Light
    {
        DirectionLightComponent();
        glm::vec3 direction;
    };

    struct SYN_API DirectionLightGPU
    {
        DirectionLightGPU(const DirectionLightComponent& component);

        glm::vec3 direction;
        float strength;
        glm::vec3 color;
        uint32_t flags;
    };
}