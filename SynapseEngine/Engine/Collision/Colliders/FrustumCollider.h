#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API FrustumCollider
    {
        glm::vec4 planes[6];

        FrustumCollider() {
            for (int i = 0; i < 6; ++i) {
                planes[i] = glm::vec4(0.0f);
            }
        }

        void Update(const glm::mat4& viewProj);

        SYN_INLINE static glm::vec4 NormalizePlane(const glm::vec4& p) {
            float length = glm::length(glm::vec3(p));
            if (length < 0.0001f) return glm::vec4(0.0f);
            return glm::vec4(p.x / length, p.y / length, p.z / length, -p.w / length);
        }

        SYN_INLINE static glm::vec4 CreatePlane(const glm::vec3& normal, const glm::vec3& point) {
            glm::vec3 n = glm::normalize(normal);
            return glm::vec4(n, glm::dot(n, point));
        }
    };
}