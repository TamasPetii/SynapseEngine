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
#include "Engine/Scene/Scene.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DHAC/Pipeline/ForwardPlusPipelineDHAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/VFC/Pipeline/ForwardPlusPipelineVFC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBC/Pipeline/ForwardPlusPipelineDBC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBAC/Pipeline/ForwardPlusPipelineDBAC.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Techniques/DBBC/Pipeline/ForwardPlusPipelineDBBC.h"

namespace Syn 
{
    ForwardPlusClusteringPipeline::ForwardPlusClusteringPipeline()
        : RenderPipeline("ForwardPlusClusteringPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<ForwardPlusPipelineDHAC>());
        AddPass(std::make_unique<ForwardPlusPipelineDBAC>());
        AddPass(std::make_unique<ForwardPlusPipelineDBBC>());
        AddPass(std::make_unique<ForwardPlusPipelineDBC>());
        AddPass(std::make_unique<ForwardPlusPipelineVFC>());
    }

    bool ForwardPlusClusteringPipeline::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.pipelineType == PipelineType::ForwardPlus;
    }
}