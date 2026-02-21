#include "MeshoptimizerMeshletProcessor.h"
#include <meshoptimizer.h>

namespace Syn
{
    MeshoptimizerMeshletProcessor::MeshoptimizerMeshletProcessor(size_t maxVertices, size_t maxTriangles, float coneWeight)
        : _maxVertices(maxVertices), _maxTriangles(maxTriangles), _coneWeight(coneWeight)
    {}

    void MeshoptimizerMeshletProcessor::Process(CookedModel& cookedModel)
    {
        for (auto& mesh : cookedModel.meshes)
        {
            if (mesh.vertices.empty() || mesh.lods.empty())
                continue;

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
        }
    }
}