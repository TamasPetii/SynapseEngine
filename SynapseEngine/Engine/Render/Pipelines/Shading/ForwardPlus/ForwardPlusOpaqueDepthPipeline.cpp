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

#include "ForwardPlusOpaqueDepthPipeline.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/OpaqueDepthTransitionPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletOpaqueDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalOpaqueDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletOpaqueAlphaTestedDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalOpaqueAlphaTestedDepthPrepass.h"

namespace Syn {
    ForwardPlusOpaqueDepthPipeline::ForwardPlusOpaqueDepthPipeline()
        : RenderPipeline("ForwardPlusOpaqueDepthPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<OpaqueDepthTransitionPrepass>());
        AddPass(std::make_unique<MeshletOpaqueDepthPrepass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<MeshletOpaqueDepthPrepass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<TraditionalOpaqueDepthPrepass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<TraditionalOpaqueDepthPrepass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<MeshletOpaqueAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<MeshletOpaqueAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<TraditionalOpaqueAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<TraditionalOpaqueAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedOpaque2Sided));
    }

    bool ForwardPlusOpaqueDepthPipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}