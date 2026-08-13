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

#include "SpotLightShadowComponent.h"

namespace Syn
{
    SpotLightShadowComponent::SpotLightShadowComponent() :
        nearPlane(0.1f),
        farPlane(15.0f),
        viewProj(1.0f),
        atlasRect(0.0f)
    {}

    SpotLightShadowComponentGPU::SpotLightShadowComponentGPU(const SpotLightShadowComponent& component) :
        planes(component.nearPlane, component.farPlane, 0.0f, 0.0f),
        view(component.view),
        proj(component.proj),
        viewProj(component.viewProj),
        atlasRect(component.atlasRect)
    {}
}