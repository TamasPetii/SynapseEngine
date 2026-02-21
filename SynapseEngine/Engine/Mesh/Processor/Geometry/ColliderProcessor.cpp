#include "ColliderProcessor.h"
#include <meshoptimizer.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

namespace Syn
{
    void ColliderProcessor::Process(CookedModel& cookedModel)
    {
        for (auto& mesh : cookedModel.meshes)
        {
            if (mesh.vertices.empty())
                continue;

            ComputeMeshLocalBounds(mesh);
            ComputeMeshletBounds(mesh);
        }

        ComputeGlobalBounds(cookedModel);
    }

    void ColliderProcessor::ComputeMeshLocalBounds(CookedMesh& mesh)
    {
        glm::vec3 minAabb(std::numeric_limits<float>::max());
        glm::vec3 maxAabb(std::numeric_limits<float>::lowest());

        for (const auto& v : mesh.vertices)
        {
            minAabb = glm::min(minAabb, v.position);
            maxAabb = glm::max(maxAabb, v.position);
        }

        mesh.collider.aabb.min = minAabb;
        mesh.collider.aabb.max = maxAabb;

        mesh.collider.sphere.center = (minAabb + maxAabb) * 0.5f;

        float maxRadiusSq = 0.0f;
        for (const auto& v : mesh.vertices)
        {
            float distSq = glm::length2(v.position - mesh.collider.sphere.center);
            if (distSq > maxRadiusSq)
                maxRadiusSq = distSq;
        }
        mesh.collider.sphere.radius = std::sqrt(maxRadiusSq);
    }

    void ColliderProcessor::ComputeMeshletBounds(CookedMesh& mesh)
    {
        for (auto& lod : mesh.lods)
        {
            for (auto& meshlet : lod.meshlets)
            {
                //Todo: Meshopt?? Need a factory that connects ColliderProcessor with MeshletProcessor!

                meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                    &lod.meshletVertexIndices[meshlet.vertexOffset],
                    &lod.meshletTriangleIndices[meshlet.triangleOffset],
                    meshlet.triangleCount,
                    &mesh.vertices[0].position.x,
                    mesh.vertices.size(),
                    sizeof(Vertex)
                );

                meshlet.collider.sphere.center = glm::vec3(bounds.center[0], bounds.center[1], bounds.center[2]);
                meshlet.collider.sphere.radius = bounds.radius;

                meshlet.collider.cone.apex = glm::vec3(bounds.cone_apex[0], bounds.cone_apex[1], bounds.cone_apex[2]);
                meshlet.collider.cone.axis = glm::vec3(bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2]);
                meshlet.collider.cone.cutoff = bounds.cone_cutoff;

                glm::vec3 mMin(std::numeric_limits<float>::max());
                glm::vec3 mMax(std::numeric_limits<float>::lowest());

                for (uint32_t i = 0; i < meshlet.vertexCount; ++i)
                {
                    uint32_t vertexIndex = lod.meshletVertexIndices[meshlet.vertexOffset + i];
                    const glm::vec3& pos = mesh.vertices[vertexIndex].position;

                    mMin = glm::min(mMin, pos);
                    mMax = glm::max(mMax, pos);
                }

                meshlet.collider.aabb.min = mMin;
                meshlet.collider.aabb.max = mMax;
            }
        }
    }

    void ColliderProcessor::ComputeGlobalBounds(CookedModel& cookedModel)
    {
        glm::vec3 globalMin(std::numeric_limits<float>::max());
        glm::vec3 globalMax(std::numeric_limits<float>::lowest());
        bool hasGlobalData = false;

        for (const auto& desc : cookedModel.meshNodeDescriptors)
        {
            if (desc.meshIndex >= cookedModel.meshes.size() || desc.nodeIndex >= cookedModel.nodeTransforms.size())
                continue;

            const CookedMesh& mesh = cookedModel.meshes[desc.meshIndex];
            const glm::mat4& transform = cookedModel.nodeTransforms[desc.nodeIndex].globalTransform;

            glm::vec3 localMin = mesh.collider.aabb.min;
            glm::vec3 localMax = mesh.collider.aabb.max;

            glm::vec3 corners[8] = {
                {localMin.x, localMin.y, localMin.z},
                {localMax.x, localMin.y, localMin.z},
                {localMin.x, localMax.y, localMin.z},
                {localMax.x, localMax.y, localMin.z},
                {localMin.x, localMin.y, localMax.z},
                {localMax.x, localMin.y, localMax.z},
                {localMin.x, localMax.y, localMax.z},
                {localMax.x, localMax.y, localMax.z}
            };

            for (int i = 0; i < 8; ++i)
            {
                glm::vec3 transformedCorner = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
                globalMin = glm::min(globalMin, transformedCorner);
                globalMax = glm::max(globalMax, transformedCorner);
            }

            hasGlobalData = true;
        }

        if (hasGlobalData)
        {
            cookedModel.globalCollider.aabb.min = globalMin;
            cookedModel.globalCollider.aabb.max = globalMax;

            glm::vec3 globalCenter = (globalMin + globalMax) * 0.5f;
            cookedModel.globalCollider.sphere.center = globalCenter;

            float maxRadiusSq = 0.0f;
            for (const auto& desc : cookedModel.meshNodeDescriptors)
            {
                const CookedMesh& mesh = cookedModel.meshes[desc.meshIndex];
                const glm::mat4& transform = cookedModel.nodeTransforms[desc.nodeIndex].globalTransform;

                for (const auto& v : mesh.vertices)
                {
                    glm::vec3 worldPos = glm::vec3(transform * glm::vec4(v.position, 1.0f));
                    float distSq = glm::length2(worldPos - globalCenter);
                    if (distSq > maxRadiusSq)
                        maxRadiusSq = distSq;
                }
            }
            cookedModel.globalCollider.sphere.radius = std::sqrt(maxRadiusSq);
        }
    }
}