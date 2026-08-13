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

#include "SpotLightComponent.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Syn
{
    SpotLightComponent::SpotLightComponent() :
        Light(),
        position(0.0f),
        direction(0.0f, 0.0f, -1.0f),
        range(15.0f),
        weakenDistance(0.0f),
        innerAngle(30.0f),
        outerAngle(45.0f)
    {}

    SpotLightComponentGPU::SpotLightComponentGPU(const SpotLightComponent& component) :
        transform(component.transform),
        position(component.position),
        range(component.range),
        direction(component.direction),
        weakenDistance(component.weakenDistance),
        color(component.color),
        strength(component.strength),
        innerAngle(component.innerAngle),
        outerAngle(component.outerAngle),
        innerCosAngle(std::cos(glm::radians(component.innerAngle))),
        outerCosAngle(std::cos(glm::radians(component.outerAngle))),
        flags(0),
        padding0(0),
        padding1(0),
        padding2(0)
    {
        if (component.useShadow) flags |= (1u << 0);
    }

    SpotLightColliderGPU::SpotLightColliderGPU(const SpotLightComponent& component, uint32_t entityId) :
        center(component.sphereCollider.center),
        radius(component.sphereCollider.radius),
        aabbMin(component.aabbCollider.min),
        entityIndex(entityId),
        aabbMax(component.aabbCollider.max),
        range(component.range),
        worldPos(component.position),
        outerAngleCos(std::cos(glm::radians(component.outerAngle))),
        worldDir(component.direction),
        outerAngleSin(std::sin(glm::radians(component.outerAngle)))
    {}
}