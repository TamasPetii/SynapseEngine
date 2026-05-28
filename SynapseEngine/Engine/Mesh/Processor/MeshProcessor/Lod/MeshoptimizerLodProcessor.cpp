#include "MeshoptimizerLodProcessor.h"
#include <meshoptimizer.h>
#include <cmath>
#include "Engine/ServiceLocator.h"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    MeshoptimizerLodProcessor::MeshoptimizerLodProcessor(uint32_t maxLods, float targetError)
        : _maxLods(maxLods), _targetError(targetError)
    {}

    void MeshoptimizerLodProcessor::Process(CookedModel& cookedModel)
    {
        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each(cookedModel.meshes.begin(), cookedModel.meshes.end(),
            [&](CookedMesh& mesh) {
                if (mesh.vertices.empty() || mesh.lods.empty() || mesh.lods[0].indices.empty())
                    return;

                float reductionFactor = 0.5f;

                for (uint32_t i = 1; i < _maxLods; ++i)
                {
                    //Cannot cache reference outside of the look, dangling pointer!
                    const std::vector<uint32_t>& baseIndices = mesh.lods[0].indices;
                    size_t baseIndexCount = baseIndices.size();
                    size_t targetIndexCount = static_cast<size_t>(baseIndexCount * std::pow(reductionFactor, i));

                    //Todo: Too small??

                    std::vector<uint32_t> lodIndices(baseIndexCount);
                    float lodError = 0.0f;

                    size_t newIndexCount = meshopt_simplify(
                        lodIndices.data(),
                        baseIndices.data(),
                        baseIndexCount,
                        &mesh.vertices[0].position.x,
                        mesh.vertices.size(),
                        sizeof(Vertex),
                        targetIndexCount,
                        _targetError,
                        0,
                        &lodError
                    );

                    if (newIndexCount == baseIndexCount || newIndexCount == 0)
                        break;

                    lodIndices.resize(newIndexCount);

                    CookedMeshLod newLod{};
                    newLod.indices = std::move(lodIndices);
                    mesh.lods.push_back(std::move(newLod));
                }
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
    }
}