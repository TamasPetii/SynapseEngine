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
#include <glm/glm.hpp>
#include <variant>

namespace Syn
{
    struct SetSpotLightColorIntent
    {
        glm::vec3 color;
        bool isDragging;
    };
    
    struct SetSpotLightStrengthIntent
    {
        float strength;
        bool isDragging;
    };

    struct SetSpotLightUseShadowIntent
    {
        bool useShadow;
    };

    struct SetSpotLightRangeIntent
    {
        float range;
        bool isDragging;
    };

    struct SetSpotLightWeakenIntent
    {
        float distance;
        bool isDragging;
    };

    struct SetSpotLightInnerAngleIntent
    {
        float angle;
        bool isDragging;
    };

    struct SetSpotLightOuterAngleIntent
    {
        float angle;
        bool isDragging;
    };

    struct SetSpotLightShadowNearIntent
    {
        float nearPlane;
        bool isDragging;
    };

    struct SetSpotLightShadowFarIntent
    {
        float farPlane;
        bool isDragging;
    };

    using SpotLightIntent = std::variant<
        SetSpotLightColorIntent, SetSpotLightStrengthIntent, SetSpotLightUseShadowIntent,
        SetSpotLightRangeIntent, SetSpotLightWeakenIntent, SetSpotLightInnerAngleIntent,
        SetSpotLightOuterAngleIntent, SetSpotLightShadowNearIntent, SetSpotLightShadowFarIntent>;
}