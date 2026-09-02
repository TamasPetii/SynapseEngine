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

#include "BillboardPipeline.h"
#include "Engine/Render/Passes/Billboard/BillboardTransitionPass.h"
#include "Engine/Render/Passes/Billboard/CameraBillboardPass.h"
#include "Engine/Render/Passes/Billboard/DirectionLightBillboardPass.h"
#include "Engine/Render/Passes/Billboard/PointLightBillboardPass.h"
#include "Engine/Render/Passes/Billboard/SpotLightBillboardPass.h"
#include "Engine/Render/Passes/Billboard/AudioListenerBillboardPass.h"
#include "Engine/Render/Passes/Billboard/AudioSourceBillboardPass.h"

namespace Syn {
    BillboardPipeline::BillboardPipeline()
        : RenderPipeline("BillboardPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<BillboardTransitionPass>());
        AddPass(std::make_unique<CameraBillboardPass>());
        AddPass(std::make_unique<DirectionLightBillboardPass>());
        AddPass(std::make_unique<PointLightBillboardPass>());
        AddPass(std::make_unique<SpotLightBillboardPass>());
        AddPass(std::make_unique<AudioSourceBillboardPass>());
        AddPass(std::make_unique<AudioListenerBillboardPass>());
    }

    bool BillboardPipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}