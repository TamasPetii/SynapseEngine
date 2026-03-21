#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API VertexSkinData
    {
        glm::uvec4 boneIndices = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
        glm::vec4 boneWeights = { 0.f, 0.f, 0.f, 0.f };
    };
}