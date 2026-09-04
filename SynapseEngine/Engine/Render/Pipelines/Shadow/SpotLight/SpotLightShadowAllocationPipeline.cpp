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

#include "SpotLightShadowAllocationPipeline.h"
#include "Engine/Render/Passes/Culling/SpotLight/SpotLightShadowBufferResetPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/SpotLightShadowCullingCommandResetPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/SpotLightCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/SpotLightShadowAtlasRadixSortPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/SpotLightShadowAtlasAllocatorPass.h"

namespace Syn {
    SpotLightShadowAllocationPipeline::SpotLightShadowAllocationPipeline()
        : RenderPipeline("SpotLightShadowAllocationPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<SpotLightShadowBufferResetPass>());
        AddPass(std::make_unique<SpotLightShadowCullingCommandResetPass>());
        AddPass(std::make_unique<SpotLightCullingPass>());
        AddPass(std::make_unique<SpotLightShadowAtlasRadixSortPass>());
        AddPass(std::make_unique<SpotLightShadowAtlasAllocatorPass>());
    }
}