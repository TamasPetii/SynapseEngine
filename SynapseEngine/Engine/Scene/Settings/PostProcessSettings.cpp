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

#include "PostProcessSettings.h"

namespace Syn
{
    PostProcessSettings::PostProcessSettings()
        : enableBloom(true)
        , bloomThreshold(1.0f)
        , bloomKnee(0.1f)
        , bloomFilterRadius(0.005f)
        , bloomExposure(1.0f)
        , bloomStrength(1.0f)
        , enableSsao(false)
        , enableSsaoLight(false)
        , aoRadius(0.95f)
        , aoIntensity(5.0f)
        , maxOcclusionDistance(10.0f)
        , depthSharpness(0.0f)
        , bias(0.05f)
        , sampleCount(64)
    {}
}