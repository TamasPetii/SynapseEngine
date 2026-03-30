#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API Light
    {
        Light();

        glm::vec3 color;
        float strength;
        bool useShadow;
    };
}