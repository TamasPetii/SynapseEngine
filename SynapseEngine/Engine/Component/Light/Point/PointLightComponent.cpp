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

#include "PointLightComponent.h"

namespace Syn
{
    PointLightComponent::PointLightComponent() :
        Light(),
        position(0.0f),
        radius(10.0f),
        weakenDistance(0.0f)
    {}

    PointLightComponentGPU::PointLightComponentGPU(const PointLightComponent& component) :
        position(component.position),
        radius(component.radius),
        color(component.color),
        strength(component.strength),
        weakenDistance(component.weakenDistance),
        flags(0),
        padding(0),
        padding2(0)
    {
        if (component.useShadow) flags |= (1u << 0);
    }

    PointLightColliderGPU::PointLightColliderGPU(const PointLightComponent& component, uint32_t entityId) :
        center(component.position),
        radius(component.radius),
        padding(0.0f),
        entityIndex(entityId)
    {}
}