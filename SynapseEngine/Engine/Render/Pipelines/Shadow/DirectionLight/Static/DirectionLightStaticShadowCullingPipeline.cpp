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

#include "DirectionLightStaticShadowCullingPipeline.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowCullingCommandResetPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowMortonChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowMortonModelCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowStaticChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowStaticModelCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowModelCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowMeshCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowAnimatedExtractionPass.h"

namespace Syn {
    DirectionLightStaticShadowCullingPipeline::DirectionLightStaticShadowCullingPipeline()
        : RenderPipeline("DirectionLightStaticShadowCullingPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DirectionLightShadowCullingCommandResetPass>(true));
        AddPass(std::make_unique<DirectionLightShadowAnimatedExtractionPass>());
        AddPass(std::make_unique<DirectionLightShadowMortonChunkCullingPass>());
        AddPass(std::make_unique<DirectionLightShadowMortonModelCullingPass>());
        AddPass(std::make_unique<DirectionLightShadowStaticChunkCullingPass>());
        AddPass(std::make_unique<DirectionLightShadowStaticModelCullingPass>());
        AddPass(std::make_unique<DirectionLightShadowModelCullingPass>(true, 0));
        AddPass(std::make_unique<DirectionLightShadowMeshCullingPass>(true));
    }
}