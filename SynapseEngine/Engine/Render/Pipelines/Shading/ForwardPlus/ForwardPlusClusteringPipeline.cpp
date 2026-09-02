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

#include "ForwardPlusClusteringPipeline.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSetupPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterDispatchSetupPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightSinglePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightSinglePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPrefixSumPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterLightWriteSyncPass.h"

namespace Syn {
    ForwardPlusClusteringPipeline::ForwardPlusClusteringPipeline()
        : RenderPipeline("ForwardPlusClusteringPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ClusterSetupPass>());
        AddPass(std::make_unique<ClusterDispatchSetupPass>());
        AddPass(std::make_unique<ClusterPointLightSinglePass>());
        AddPass(std::make_unique<ClusterSpotLightSinglePass>());
        AddPass(std::make_unique<ClusterPointLightCountPass>());
        AddPass(std::make_unique<ClusterSpotLightCountPass>());
        AddPass(std::make_unique<ClusterPrefixSumPass>());
        AddPass(std::make_unique<ClusterPointLightWritePass>());
        AddPass(std::make_unique<ClusterSpotLightWritePass>());
        AddPass(std::make_unique<ClusterLightWriteSyncPass>());
    }
}