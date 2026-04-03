#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API AnimationNode
    {
        std::string name;
        uint32_t parentIndex = UINT32_MAX;
        uint32_t trackIndex = UINT32_MAX;
        glm::mat4 offsetMatrix = glm::mat4(1.0f);
        glm::mat4 defaultLocalTransform = glm::mat4(1.0f);
    };
}