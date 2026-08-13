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

#include "MeshoptimizerOptimizerProcessor.h"
#include <meshoptimizer.h>
#include "Engine/ServiceLocator.h"

#include <chrono>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    MeshoptimizerOptimizerProcessor::MeshoptimizerOptimizerProcessor(float overdrawThreshold)
        : _overdrawThreshold(overdrawThreshold)
    {}

    void MeshoptimizerOptimizerProcessor::Process(CookedModel& cookedModel)
    {
        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each(cookedModel.meshes.begin(), cookedModel.meshes.end(),
            [&](CookedMesh& mesh) {
                if (mesh.vertices.empty() || mesh.lods.empty())
                    return;

                for (auto& lod : mesh.lods)
                {
                    if (lod.indices.empty())
                        continue;

                    meshopt_optimizeVertexCache(
                        lod.indices.data(),
                        lod.indices.data(),
                        lod.indices.size(),
                        mesh.vertices.size()
                    );

                    meshopt_optimizeOverdraw(
                        lod.indices.data(),
                        lod.indices.data(),
                        lod.indices.size(),
                        &mesh.vertices[0].position.x,
                        mesh.vertices.size(),
                        sizeof(Syn::Vertex),
                        _overdrawThreshold
                    );
                }

                std::vector<unsigned int> remap(mesh.vertices.size());
                const auto& lod0Indices = mesh.lods[0].indices;

                size_t newVertexCount = meshopt_optimizeVertexFetchRemap(
                    remap.data(),
                    lod0Indices.data(),
                    lod0Indices.size(),
                    mesh.vertices.size()
                );

                std::vector<Syn::Vertex> remappedVertices(newVertexCount);

                meshopt_remapVertexBuffer(
                    remappedVertices.data(),
                    mesh.vertices.data(),
                    mesh.vertices.size(),
                    sizeof(decltype(mesh.vertices[0])),
                    remap.data()
                );

                mesh.vertices = std::move(remappedVertices);

                for (auto& lod : mesh.lods)
                {
                    if (lod.indices.empty())
                        continue;

                    meshopt_remapIndexBuffer(
                        lod.indices.data(),
                        lod.indices.data(),
                        lod.indices.size(),
                        remap.data()
                    );
                }
            },
            partitioner
        );

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
    }
}