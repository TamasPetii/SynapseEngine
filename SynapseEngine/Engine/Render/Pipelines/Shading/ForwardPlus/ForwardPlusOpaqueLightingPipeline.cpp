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

#include "ForwardPlusOpaqueLightingPipeline.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/OpaqueForwardTransitionPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/MeshletOpaqueForwardPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/TraditionalOpaqueForwardPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/MeshletOpaqueAlphaTestedForwardPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/TraditionalOpaqueAlphaTestedForwardPass.h"

namespace Syn {
    ForwardPlusOpaqueLightingPipeline::ForwardPlusOpaqueLightingPipeline()
        : RenderPipeline("ForwardPlusOpaqueLightingPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<OpaqueForwardTransitionPass>());
        AddPass(std::make_unique<MeshletOpaqueForwardPass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<MeshletOpaqueForwardPass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<TraditionalOpaqueForwardPass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<TraditionalOpaqueForwardPass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<MeshletOpaqueAlphaTestedForwardPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<MeshletOpaqueAlphaTestedForwardPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<TraditionalOpaqueAlphaTestedForwardPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<TraditionalOpaqueAlphaTestedForwardPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
    }

    bool ForwardPlusOpaqueLightingPipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}