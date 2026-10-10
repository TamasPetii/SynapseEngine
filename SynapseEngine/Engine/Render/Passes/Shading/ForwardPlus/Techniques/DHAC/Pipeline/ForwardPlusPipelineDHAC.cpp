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

#include "ForwardPlusPipelineDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterSetupPassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterDispatchSetupPassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterPointLightSinglePassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterSpotLightSinglePassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterPointLightCountPassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterSpotLightCountPassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterPrefixSumPassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterPointLightWritePassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterSpotLightWritePassDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Passes/ClusterLightWriteSyncPassDHAC.h"

namespace Syn {
    ForwardPlusPipelineDHAC::ForwardPlusPipelineDHAC()
        : RenderPipeline("ForwardPlusPipelineDHAC", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterSetupPassDHAC>());
        AddPass(std::make_unique<ClusterDispatchSetupPassDHAC>());
        AddPass(std::make_unique<ClusterPointLightSinglePassDHAC>());
        AddPass(std::make_unique<ClusterSpotLightSinglePassDHAC>());
        AddPass(std::make_unique<ClusterPointLightCountPassDHAC>());
        AddPass(std::make_unique<ClusterSpotLightCountPassDHAC>());
        AddPass(std::make_unique<ClusterPrefixSumPassDHAC>());
        AddPass(std::make_unique<ClusterPointLightWritePassDHAC>());
        AddPass(std::make_unique<ClusterSpotLightWritePassDHAC>());
        AddPass(std::make_unique<ClusterLightWriteSyncPassDHAC>());
    }

    bool ForwardPlusPipelineDHAC::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::DHAC;
    }
}