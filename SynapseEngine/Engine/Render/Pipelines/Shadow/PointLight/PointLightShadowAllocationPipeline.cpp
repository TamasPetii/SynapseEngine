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

#include "PointLightShadowAllocationPipeline.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowBufferResetPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowCullingCommandResetPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowAtlasRadixSortPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowAtlasAllocatorPass.h"

namespace Syn {
    PointLightShadowAllocationPipeline::PointLightShadowAllocationPipeline()
        : RenderPipeline("PointLightShadowAllocationPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<PointLightShadowBufferResetPass>());
        AddPass(std::make_unique<PointLightShadowCullingCommandResetPass>());
        AddPass(std::make_unique<PointLightCullingPass>());
        AddPass(std::make_unique<PointLightShadowAtlasRadixSortPass>());
        AddPass(std::make_unique<PointLightShadowAtlasAllocatorPass>());
    }
}