#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/BitFlag.h"
#include <glm/glm.hpp>

namespace Syn
{
    constexpr uint32_t SHADOW_TOGGLE_BIT = CUSTOM_CHANGED_BIT1;

    struct SYN_API Light
    {
        Light();

        glm::vec3 color;
        float strength;
        bool useShadow;
    };
}