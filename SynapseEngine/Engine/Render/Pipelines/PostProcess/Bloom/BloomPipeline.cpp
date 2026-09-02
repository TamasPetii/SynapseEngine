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

#include "BloomPipeline.h"
#include "Engine/Render/Passes/PostProcess/Bloom/BloomPrefilterPass.h"
#include "Engine/Render/Passes/PostProcess/Bloom/BloomDownsamplePass.h"
#include "Engine/Render/Passes/PostProcess/Bloom/BloomUpsamplePass.h"
#include "Engine/Render/Passes/PostProcess/Bloom/BloomCompositePass.h"

namespace Syn {
    BloomPipeline::BloomPipeline()
        : RenderPipeline("BloomPhase", PassGroupNames::PostProcessPasses)
    {
        AddPass(std::make_unique<BloomPrefilterPass>());
        AddPass(std::make_unique<BloomDownsamplePass>());
        AddPass(std::make_unique<BloomUpsamplePass>());
        AddPass(std::make_unique<BloomCompositePass>());
    }

    bool BloomPipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}