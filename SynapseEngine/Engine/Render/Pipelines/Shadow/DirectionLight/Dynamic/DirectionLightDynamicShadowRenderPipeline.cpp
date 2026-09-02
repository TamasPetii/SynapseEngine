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

#include "DirectionLightDynamicShadowRenderPipeline.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowAtlasRestorePass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTraditionalOpaquePass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowMeshletOpaquePass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowMeshletOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTransparentTransitionPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTraditionalTransparentPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowMeshletTransparentPass.h"

namespace Syn {
    DirectionLightDynamicShadowRenderPipeline::DirectionLightDynamicShadowRenderPipeline()
        : RenderPipeline("DirectionLightDynamicShadowRenderPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DirectionLightShadowAtlasRestorePass>());
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowTransparentTransitionPass>());
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
    }
}