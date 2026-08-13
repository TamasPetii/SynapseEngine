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

#include "LightingSettings.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn
{
    LightingSettings::LightingSettings()
        : pipelineType(PipelineType::ForwardPlus)
        , tileSize(ComputeGroupSize::Image64D)
        , msaaSamples(4)
        , ambientStrength(0.05f)
        , emissiveStrength(1.00f)
        , enableDeferredEmissiveAo(true)
        , enableDeferredPointLights(true)
        , enableDeferredSpotLights(true)
        , enableDeferredDirectionalLights(true)
        , enableForwardPlusEmissiveAo(true)
        , enableForwardPlusPointLights(true)
        , enableForwardPlusSpotLights(true)
        , enableForwardPlusDirectionalLights(true)
    {}
}