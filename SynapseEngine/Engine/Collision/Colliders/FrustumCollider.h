#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API FrustumCollider
    {
        // 0 = Near, 1 = Far, 2 = Left, 3 = Right, 4 = Top, 5 = Bottom
        glm::vec4 planes[6];

        FrustumCollider() = default;

        void Update(const glm::mat4& viewProj);

        SYN_INLINE static glm::vec4 NormalizePlane(const glm::vec4& p) {
            float length = glm::length(glm::vec3(p));
            return p / length;
        }

        SYN_INLINE static glm::vec4 CreatePlane(const glm::vec3& normal, const glm::vec3& point) {
            glm::vec3 n = glm::normalize(normal);
            return glm::vec4(n, -glm::dot(n, point));
        }
    };
}