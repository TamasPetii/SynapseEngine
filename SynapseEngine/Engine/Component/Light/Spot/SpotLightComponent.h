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
#include "Engine/Component/Core/Component.h"
#include "Engine/Component/Light/Light.h"
#include "Engine/Collision/Colliders/SphereCollider.h"
#include "Engine/Collision/Colliders/AabbCollider.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API SpotLightComponent : public Component, public Light
    {
        SpotLightComponent();

        glm::vec3 position;
        glm::vec3 direction;

        float range;
        float weakenDistance;
        float innerAngle;
        float outerAngle;
    private:
        glm::mat4 transform;
		AabbCollider aabbCollider;
        SphereCollider sphereCollider;
        friend struct SpotLightColliderGPU;
        friend struct SpotLightComponentGPU;
        friend class SpotLightSystem;
        friend class SpotLightCullingSystem;
        friend class SpotLightShadowAtlasSystem;
    };

    struct SYN_API SpotLightComponentGPU
    {
        SpotLightComponentGPU(const SpotLightComponent& component);

        glm::mat4 transform;

        glm::vec3 position;
        float range; 
        glm::vec3 direction;
		float weakenDistance;
        glm::vec3 color;
		float strength;

		float innerAngle;
		float outerAngle;
        float innerCosAngle;
		float outerCosAngle;

        uint32_t flags;
		uint32_t padding0;
        uint32_t padding1;
        uint32_t padding2;
    };

    struct SYN_API SpotLightColliderGPU
    {
        SpotLightColliderGPU(const SpotLightComponent& component, uint32_t entityIndex);

        glm::vec3 center;
        float radius;
        glm::vec3 aabbMin;
        uint32_t entityIndex;
        glm::vec3 aabbMax;
        float range;
        glm::vec3 worldPos;
        float outerAngleCos;
        glm::vec3 worldDir;
        float outerAngleSin;
    };
}