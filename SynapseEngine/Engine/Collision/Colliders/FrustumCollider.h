// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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