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

#include "MeshoptimizerLodProcessor.h"
#include <meshoptimizer.h>
#include <cmath>
#include "Engine/ServiceLocator.h"

#include <chrono>
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

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
    }
}