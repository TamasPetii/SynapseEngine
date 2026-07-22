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
                    glm::vec3& pos = cpuData.vertices[job.vertexOffset + i];
                    pos = glm::vec3(job.transform * glm::vec4(pos, 1.0f));
                }
            },
            partitioner
        );

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
        taskflow.clear();
    }
}