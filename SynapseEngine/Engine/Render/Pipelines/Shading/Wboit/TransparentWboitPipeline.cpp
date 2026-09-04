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

#include "TransparentWboitPipeline.h"
#include "Engine/Render/Passes/Shading/Wboit/TransparentForwardTransitionPass.h"
#include "Engine/Render/Passes/Shading/Wboit/MeshletTransparentForwardPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TraditionalTransparentForwardPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TransparentCompositeTransitionPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TransparentCompositePass.h"

namespace Syn {
    TransparentWboitPipeline::TransparentWboitPipeline()
        : RenderPipeline("TransparentWboitPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<TransparentForwardTransitionPass>());
        AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<TransparentCompositeTransitionPass>());
        AddPass(std::make_unique<TransparentCompositePass>());
    }
}