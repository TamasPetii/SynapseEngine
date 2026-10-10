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

#include "SpotLightShadowGeometryCullingPipeline.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowMortonChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowMortonModelCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowStaticChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowStaticModelCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowModelCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowMeshCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowCullingMemoryBarrierPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowRadixSortPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowFinalizeSetupPass.h"
#include "Engine/Render/Passes/Culling/SpotLight/Passes/SpotLightShadowFinalizePass.h"

namespace Syn {
    SpotLightShadowGeometryCullingPipeline::SpotLightShadowGeometryCullingPipeline()
        : RenderPipeline("SpotLightShadowGeometryCullingPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<SpotLightShadowMortonChunkCullingPass>());
        AddPass(std::make_unique<SpotLightShadowMortonModelCullingPass>());
        AddPass(std::make_unique<SpotLightShadowStaticChunkCullingPass>());
        AddPass(std::make_unique<SpotLightShadowStaticModelCullingPass>());
        AddPass(std::make_unique<SpotLightShadowModelCullingPass>());
        AddPass(std::make_unique<SpotLightShadowMeshCullingPass>());
        AddPass(std::make_unique<SpotLightShadowCullingMemoryBarrierPass>());
        AddPass(std::make_unique<SpotLightShadowRadixSortPass>());
        AddPass(std::make_unique<SpotLightShadowFinalizeSetupPass>());
        AddPass(std::make_unique<SpotLightShadowFinalizePass>());
    }
}