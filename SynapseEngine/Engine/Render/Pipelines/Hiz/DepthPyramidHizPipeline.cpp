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

#include "DepthPyramidHizPipeline.h"
#include "Engine/Render/Passes/Hiz/Geometry/GeometryHizLinearPreparePass.h"
#include "Engine/Render/Passes/Hiz/Geometry/GeometryHizDownsamplePass.h"
#include "Engine/Render/Passes/Shadow/ShadowAtlasTransitionPass.h"

namespace Syn {
    DepthPyramidHizPipeline::DepthPyramidHizPipeline()
        : RenderPipeline("DepthPyramidHizPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<GeometryHizLinearPreparePass>());
        AddPass(std::make_unique<GeometryHizDownsamplePass>());

        /*
        AddPass(std::make_unique<DirectionLightShadowHizCopyPass>());
        AddPass(std::make_unique<DirectionLightShadowHizDownsamplePass>());
        AddPass(std::make_unique<SpotLightShadowHizCopyPass>());
        AddPass(std::make_unique<SpotLightShadowHizDownsamplePass>());
        AddPass(std::make_unique<PointLightShadowHizCopyPass>());
        AddPass(std::make_unique<PointLightShadowHizDownsamplePass>());
        */

        AddPass(std::make_unique<ShadowAtlasTransitionPass>());
    }
}