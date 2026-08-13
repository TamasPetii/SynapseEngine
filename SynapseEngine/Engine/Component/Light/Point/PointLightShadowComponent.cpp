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

#include "PointLightShadowComponent.h"

namespace Syn
{
    PointLightShadowComponent::PointLightShadowComponent() :
        nearPlane(0.1f),
        farPlane(10.0f)
    {
        viewProjs.fill(glm::mat4(1.0f));
        atlasRects.fill(glm::vec4(0.0f));
    }

    PointLightShadowComponentGPU::PointLightShadowComponentGPU(const PointLightShadowComponent& component) :
        planes(component.nearPlane, component.farPlane, 0.0f, 0.0f),
        mainAtlasRect(component.mainAtlasRect)
    {
        for (int i = 0; i < 6; ++i)
        {
            viewProjs[i] = component.viewProjs[i];
            atlasRects[i] = component.atlasRects[i];
        }
    }
}