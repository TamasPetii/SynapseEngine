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

#include "ForwardPlusTransparentDepthPipeline.h"
#include "Engine/Render/Passes/Shading/Common/DepthCopyPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TransparentDepthTransitionPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletTransparentDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalTransparentDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletTransparentAlphaTestedDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalTransparentAlphaTestedDepthPrepass.h"

namespace Syn {
    ForwardPlusTransparentDepthPipeline::ForwardPlusTransparentDepthPipeline()
        : RenderPipeline("ForwardPlusTransparentDepthPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DepthCopyPass>());
        AddPass(std::make_unique<TransparentDepthTransitionPrepass>());
        AddPass(std::make_unique<MeshletTransparentDepthPrepass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<MeshletTransparentDepthPrepass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<TraditionalTransparentDepthPrepass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<TraditionalTransparentDepthPrepass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<MeshletTransparentAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<MeshletTransparentAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<TraditionalTransparentAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<TraditionalTransparentAlphaTestedDepthPrepass>(MaterialRenderType::AlphaTestedTransparent2Sided));
    }
}