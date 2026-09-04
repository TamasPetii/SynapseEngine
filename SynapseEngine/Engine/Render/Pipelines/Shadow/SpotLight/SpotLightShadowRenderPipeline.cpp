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

#include "SpotLightShadowRenderPipeline.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowInitPass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowTraditionalOpaquePass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowMeshletOpaquePass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowMeshletOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowTransparentTransitionPass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowTraditionalTransparentPass.h"
#include "Engine/Render/Passes/Shadow/SpotLight/SpotLightShadowMeshletTransparentPass.h"

namespace Syn {
    SpotLightShadowRenderPipeline::SpotLightShadowRenderPipeline()
        : RenderPipeline("SpotLightShadowRenderPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<SpotLightShadowInitPass>());
        AddPass(std::make_unique<SpotLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<SpotLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<SpotLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<SpotLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<SpotLightShadowTransparentTransitionPass>());
        AddPass(std::make_unique<SpotLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<SpotLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<SpotLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<SpotLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<SpotLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
    }
}