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

#include "VertexTransformProcessor.h"
#include "Engine/ServiceLocator.h"

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    struct VertexTransformJob {
        uint32_t vertexOffset;
        uint32_t vertexCount;
        glm::mat4 transform;
    };

    void VertexTransformProcessor::Process(CpuModelData& cpuData)
    {
        if (cpuData.vertices.empty() || cpuData.nodeTransforms.empty()) return;

        if (cpuData.worldPositions.size() != cpuData.vertices.size()) {
            cpuData.worldPositions.resize(cpuData.vertices.size());
        }

        std::vector<VertexTransformJob> jobs;
        jobs.reserve(cpuData.meshDescriptors.size() / 4);

        uint32_t validMeshCounter = 0;

        for (const auto& desc : cpuData.meshNodeDescriptors)
        {
            if (desc.meshIndex == 0xFFFF) continue;

            uint32_t lod0Index = validMeshCounter * 4;

            if (lod0Index < cpuData.meshDescriptors.size() && desc.nodeIndex < cpuData.nodeTransforms.size())
            {
                const auto& meshDesc = cpuData.meshDescriptors[lod0Index];
                jobs.push_back({
                    meshDesc.vertexOffset,
                    meshDesc.vertexCount,
                    cpuData.nodeTransforms[desc.nodeIndex]
                    });
            }
            validMeshCounter++;
        }

        if (jobs.empty()) return;

        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each(jobs.begin(), jobs.end(),
            [&cpuData](const VertexTransformJob& job) {
                for (uint32_t i = 0; i < job.vertexCount; ++i)
                {
                    const glm::vec3& localPos = cpuData.vertices[job.vertexOffset + i];
                    cpuData.worldPositions[job.vertexOffset + i] = glm::vec3(job.transform * glm::vec4(localPos, 1.0f));
                }
            },
            partitioner
        );

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
        taskflow.clear();
    }
}