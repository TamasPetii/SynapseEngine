#pragma once
#include "Engine/SynApi.h"
#include "Engine/Component/Core/Component.h"
#include "Engine/Component/Light/Light.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API DirectionLightComponent : public Component, public Light
    {
        DirectionLightComponent();
        glm::vec3 direction;
    };

    struct SYN_API DirectionLightComponentGPU
    {
        DirectionLightComponentGPU(const DirectionLightComponent& component);

        glm::vec3 direction;
        float strength;
        glm::vec3 color;
        uint32_t flags;
    };
}