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

#include "PointLightShadowGeometryCullingPipeline.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowMortonChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowMortonModelCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowStaticChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowStaticModelCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowModelCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowMeshCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowCullingMemoryBarrierPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowRadixSortPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowFinalizeSetupPass.h"
#include "Engine/Render/Passes/Culling/PointLight/PointLightShadowFinalizePass.h"

namespace Syn {
    PointLightShadowGeometryCullingPipeline::PointLightShadowGeometryCullingPipeline()
        : RenderPipeline("PointLightShadowGeometryCullingPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<PointLightShadowMortonChunkCullingPass>());
        AddPass(std::make_unique<PointLightShadowMortonModelCullingPass>());
        AddPass(std::make_unique<PointLightShadowStaticChunkCullingPass>());
        AddPass(std::make_unique<PointLightShadowStaticModelCullingPass>());
        AddPass(std::make_unique<PointLightShadowModelCullingPass>());
        AddPass(std::make_unique<PointLightShadowMeshCullingPass>());
        AddPass(std::make_unique<PointLightShadowCullingMemoryBarrierPass>());
        AddPass(std::make_unique<PointLightShadowRadixSortPass>());
        AddPass(std::make_unique<PointLightShadowFinalizeSetupPass>());
        AddPass(std::make_unique<PointLightShadowFinalizePass>());
    }
}