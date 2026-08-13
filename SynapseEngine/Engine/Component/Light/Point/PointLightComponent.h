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
#include <glm/glm.hpp>
#include <array>

namespace Syn
{
    struct SYN_API PointLightComponent : public Component, public Light
    {
        PointLightComponent();

        glm::vec3 position;
        float radius;
        float weakenDistance;
    };

    struct SYN_API PointLightComponentGPU
    {
        PointLightComponentGPU(const PointLightComponent& component);

        glm::vec3 position;
        float radius;
        glm::vec3 color;
		float strength;
        float weakenDistance;
        uint32_t flags;
        uint32_t padding;
        uint32_t padding2;
    };

    struct SYN_API PointLightColliderGPU
    {
        PointLightColliderGPU(const PointLightComponent& component, uint32_t entityId);

        glm::vec3 center;
        float radius;
        glm::vec3 padding;
        uint32_t entityIndex;
    };
}