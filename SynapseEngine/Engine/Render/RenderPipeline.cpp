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

#include "RenderPipeline.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Profiler/IGpuProfiler.h"
#include "Engine/Statistics/IRenderStatCollector.h"
#include "Engine/Shader/ShaderManager.h"

namespace Syn
{
    void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
    {
        _passes.push_back(std::move(pass));
    }

    void RenderPipeline::Initialize()
    {
        for (auto& pass : _passes) {
            pass->Initialize();
        }
    }

    void RenderPipeline::Execute(const RenderContext& context)
    {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        bool isCompiling = shaderManager->IsCompiling();

        auto profiler = ServiceLocator::Get<IGpuProfiler>();
        auto statCollector = ServiceLocator::Get<IRenderStatCollector>();

        if (context.scene)
        {
            for (auto& pass : _passes) 
            {
                if (isCompiling && !pass->CanExecuteWhileCompiling()) {
                    continue;
                }

                if (pass->ShouldExecute(context))
                {
                    bool isProfileable = pass->IsProfileable();
                    uint32_t measureIdx = 0;

                    if (isProfileable) {
                        measureIdx = profiler->StartPass(context.cmd, context.frameIndex, pass->GetGroup(), pass->GetName());
                    }

                    bool collectStats = pass->ShouldCollectStatistics();
                    uint32_t statIdx = 0;

                    if (collectStats) {
                        statIdx = statCollector->StartPass(context.cmd, context.frameIndex, pass->GetGroup(), pass->GetName());
                    }

                    pass->Execute(context);

                    if (collectStats) {
                        statCollector->EndPass(context.cmd, context.frameIndex, statIdx);
                    }

                    if (isProfileable) {
                        profiler->EndPass(context.cmd, context.frameIndex, measureIdx);
                    }
                }
            }
        }
    }
}