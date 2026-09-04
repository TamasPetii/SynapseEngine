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

#include "DeferredGBufferPipeline.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/OpaqueDeferredTransitionPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/MeshletOpaqueDeferredPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/TraditionalOpaqueDeferredPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/MeshletOpaqueAlphaTestedDeferredPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/TraditionalOpaqueAlphaTestedDeferredPass.h"

namespace Syn {
    DeferredGBufferPipeline::DeferredGBufferPipeline()
        : RenderPipeline("DeferredGBufferPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<OpaqueDeferredTransitionPass>());
        AddPass(std::make_unique<MeshletOpaqueDeferredPass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<MeshletOpaqueDeferredPass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<TraditionalOpaqueDeferredPass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<TraditionalOpaqueDeferredPass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<MeshletOpaqueAlphaTestedDeferredPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<MeshletOpaqueAlphaTestedDeferredPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<TraditionalOpaqueAlphaTestedDeferredPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<TraditionalOpaqueAlphaTestedDeferredPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
    }

    bool DeferredGBufferPipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}