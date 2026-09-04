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

#include "DirectionLightDynamicShadowCullingPipeline.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowCullingCommandResetPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowModelCullingPass.h"
#include "Engine/Render/Passes/Culling/DirectionLight/DirectionLightShadowMeshCullingPass.h"

namespace Syn {
    DirectionLightDynamicShadowCullingPipeline::DirectionLightDynamicShadowCullingPipeline()
        : RenderPipeline("DirectionLightDynamicShadowCullingPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DirectionLightShadowCullingCommandResetPass>(false));
        AddPass(std::make_unique<DirectionLightShadowModelCullingPass>(false, 1));
        AddPass(std::make_unique<DirectionLightShadowModelCullingPass>(false, 2)); //Static Animations!
        AddPass(std::make_unique<DirectionLightShadowMeshCullingPass>(false));
    }
}