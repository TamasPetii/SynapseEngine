#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Syn
{
    struct SYN_API AnimationKeyPosition { 
        float time;
        glm::vec3 value;
    };

    struct SYN_API AnimationKeyRotation { 
        float time;
        glm::quat value;
    };

    struct SYN_API AnimationKeyScale { 
        float time;
        glm::vec3 value;
    };
}