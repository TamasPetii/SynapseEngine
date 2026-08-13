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

#include "MeshoptimizerMeshletProcessor.h"
#include <meshoptimizer.h>
#include "Engine/ServiceLocator.h"

#include <chrono>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    MeshoptimizerMeshletProcessor::MeshoptimizerMeshletProcessor(size_t maxVertices, size_t maxTriangles, float coneWeight)
        : _maxVertices(maxVertices), _maxTriangles(maxTriangles), _coneWeight(coneWeight)
    {}

    void MeshoptimizerMeshletProcessor::Process(CookedModel& cookedModel)
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

                    size_t max_meshlets = meshopt_buildMeshletsBound(lod.indices.size(), _maxVertices, _maxTriangles);

                    std::vector<meshopt_Meshlet> meshoptMeshlets(max_meshlets);
                    std::vector<unsigned int> meshoptVertexIndices(max_meshlets * _maxVertices);
                    std::vector<unsigned char> meshoptTriangleIndices(max_meshlets * _maxTriangles * 3);

                    size_t meshlet_count = meshopt_buildMeshlets(
                        meshoptMeshlets.data(),
                        meshoptVertexIndices.data(),
                        meshoptTriangleIndices.data(),
                        lod.indices.data(),
                        lod.indices.size(),
                        &mesh.vertices[0].position.x,
                        mesh.vertices.size(),
                        sizeof(Vertex),
                        _maxVertices,
                        _maxTriangles,
                        _coneWeight
                    );

                    const meshopt_Meshlet& last_meshlet = meshoptMeshlets[meshlet_count - 1];

                    meshoptVertexIndices.resize(last_meshlet.vertex_offset + last_meshlet.vertex_count);
                    meshoptTriangleIndices.resize(last_meshlet.triangle_offset + ((last_meshlet.triangle_count * 3 + 3) & ~3));

                    lod.meshletVertexIndices = std::move(meshoptVertexIndices);
                    lod.meshletTriangleIndices = std::move(meshoptTriangleIndices);
                    lod.meshlets.reserve(meshlet_count);

                    for (size_t i = 0; i < meshlet_count; ++i)
                    {
                        const auto& m = meshoptMeshlets[i];
                        CookedMeshlet cookedMeshlet{};
                        cookedMeshlet.vertexOffset = m.vertex_offset;
                        cookedMeshlet.triangleOffset = m.triangle_offset;
                        cookedMeshlet.vertexCount = m.vertex_count;
                        cookedMeshlet.triangleCount = m.triangle_count;

                        lod.meshlets.push_back(cookedMeshlet);
                    }
                }
            },
            partitioner
        );

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
    }
}