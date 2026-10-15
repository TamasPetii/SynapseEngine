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
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterDispatchSetupPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightSinglePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightSinglePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterLightWriteSyncPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterPointLightPrefixSumPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/Common/ClusterSpotLightPrefixSumPass.h"

namespace Syn {
    ForwardPlusPipelineDHAC::ForwardPlusPipelineDHAC()
        : RenderPipeline("ForwardPlusPipelineDHAC", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterSetupPassDHAC>());
        AddPass(std::make_unique<ClusterDispatchSetupPass>());

        AddPass(std::make_unique<ClusterPointLightSinglePass>());
        AddPass(std::make_unique<ClusterSpotLightSinglePass>());

        AddPass(std::make_unique<ClusterPointLightCountPass>());
        AddPass(std::make_unique<ClusterPointLightPrefixSumPass>());
        AddPass(std::make_unique<ClusterPointLightWritePass>());

        AddPass(std::make_unique<ClusterSpotLightCountPass>());
        AddPass(std::make_unique<ClusterSpotLightPrefixSumPass>());
        AddPass(std::make_unique<ClusterSpotLightWritePass>());

        AddPass(std::make_unique<ClusterLightWriteSyncPass>());
    }

    bool ForwardPlusPipelineDHAC::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.forwardPlusTechnique == ForwardPlusTechnique::DHAC;
    }
}