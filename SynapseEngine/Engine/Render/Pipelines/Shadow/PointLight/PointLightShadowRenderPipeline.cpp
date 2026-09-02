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

#include "PointLightShadowRenderPipeline.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowInitPass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowTraditionalOpaquePass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowMeshletOpaquePass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowMeshletOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowTransparentTransitionPass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowTraditionalTransparentPass.h"
#include "Engine/Render/Passes/Shadow/PointLight/PointLightShadowMeshletTransparentPass.h"

namespace Syn {
    PointLightShadowRenderPipeline::PointLightShadowRenderPipeline()
        : RenderPipeline("PointLightShadowRenderPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<PointLightShadowInitPass>());
        AddPass(std::make_unique<PointLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<PointLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<PointLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<PointLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<PointLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<PointLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<PointLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<PointLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<PointLightShadowTransparentTransitionPass>());
        AddPass(std::make_unique<PointLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<PointLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<PointLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<PointLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<PointLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<PointLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<PointLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<PointLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
    }
}