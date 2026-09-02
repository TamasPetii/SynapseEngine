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

#include "DirectionLightStaticShadowPipeline.h"
#include "DirectionLightStaticShadowCullingPipeline.h"
#include "DirectionLightStaticShadowRenderPipeline.h"
#include "DirectionLightStaticShadowHizPipeline.h"

namespace Syn {
    DirectionLightStaticShadowPipeline::DirectionLightStaticShadowPipeline()
        : RenderPipeline("DirectionLightStaticShadowPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DirectionLightStaticShadowCullingPipeline>());
        AddPass(std::make_unique<DirectionLightStaticShadowRenderPipeline>());
        AddPass(std::make_unique<DirectionLightStaticShadowHizPipeline>());
    }

    bool DirectionLightStaticShadowPipeline::ShouldExecute(const RenderContext& context) const {
        if (!context.scene) return false;
        // TODO: Return true only if DirectionLight moved or static geometry changed
        return true;
    }
}