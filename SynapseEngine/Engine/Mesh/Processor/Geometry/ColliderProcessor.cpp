#include "ColliderProcessor.h"
#include <meshoptimizer.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "Engine/ServiceLocator.h"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include <limits>

namespace Syn
{
    struct MeshletJob {
        CookedMesh* mesh;
        CookedMeshLod* lod;
        CookedMeshlet* meshlet;
    };

    void ColliderProcessor::Process(CookedModel& cookedModel)
    {
        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each(cookedModel.meshes.begin(), cookedModel.meshes.end(), 
            [this](CookedMesh& mesh) {
            if (!mesh.vertices.empty()) {
                ComputeMeshLocalBounds(mesh);
            }
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
        taskflow.clear();

        std::vector<MeshletJob> meshletJobs;
        for (auto& mesh : cookedModel.meshes) {
            for (auto& lod : mesh.lods) {
                for (auto& meshlet : lod.meshlets) {
                    meshletJobs.push_back({ &mesh, &lod, &meshlet });
                }
            }
        }

        taskflow.for_each(meshletJobs.begin(), meshletJobs.end(), 
            [this](MeshletJob& job) {
            ComputeMeshletBoundsJob(*job.mesh, *job.lod, *job.meshlet);
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
        taskflow.clear();

        // (Map-Reduce)
        ComputeGlobalBounds(cookedModel, taskflow);
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

    void ColliderProcessor::ComputeMeshletBoundsJob(CookedMesh& mesh, CookedMeshLod& lod, CookedMeshlet& meshlet)
    {
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

    void ColliderProcessor::ComputeGlobalBounds(CookedModel& cookedModel, tf::Taskflow& taskflow)
    {
        if (cookedModel.meshNodeDescriptors.empty()) return;

        tf::GuidedPartitioner partitioner(1);

        size_t descCount = cookedModel.meshNodeDescriptors.size();

        std::vector<glm::vec3> nodeMins(descCount, glm::vec3(std::numeric_limits<float>::max()));
        std::vector<glm::vec3> nodeMaxs(descCount, glm::vec3(std::numeric_limits<float>::lowest()));

        // AABB MAP FÁZIS
        taskflow.for_each_index(size_t(0), descCount, size_t(1), 
            [&](size_t i) {
            const auto& desc = cookedModel.meshNodeDescriptors[i];
            if (desc.meshIndex >= cookedModel.meshes.size() || desc.nodeIndex >= cookedModel.nodeTransforms.size())
                return;

            const CookedMesh& mesh = cookedModel.meshes[desc.meshIndex];
            const glm::mat4& transform = cookedModel.nodeTransforms[desc.nodeIndex].globalTransform;

            glm::vec3 localMin = mesh.collider.aabb.min;
            glm::vec3 localMax = mesh.collider.aabb.max;

            glm::vec3 corners[8] = {
                {localMin.x, localMin.y, localMin.z}, {localMax.x, localMin.y, localMin.z},
                {localMin.x, localMax.y, localMin.z}, {localMax.x, localMax.y, localMin.z},
                {localMin.x, localMin.y, localMax.z}, {localMax.x, localMin.y, localMax.z},
                {localMin.x, localMax.y, localMax.z}, {localMax.x, localMax.y, localMax.z}
            };

            glm::vec3 nMin(std::numeric_limits<float>::max());
            glm::vec3 nMax(std::numeric_limits<float>::lowest());

            for (int c = 0; c < 8; ++c) {
                glm::vec3 transformedCorner = glm::vec3(transform * glm::vec4(corners[c], 1.0f));
                nMin = glm::min(nMin, transformedCorner);
                nMax = glm::max(nMax, transformedCorner);
            }

            nodeMins[i] = nMin;
            nodeMaxs[i] = nMax;
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
        taskflow.clear();

        //AABB REDUCE
        glm::vec3 globalMin(std::numeric_limits<float>::max());
        glm::vec3 globalMax(std::numeric_limits<float>::lowest());
        for (size_t i = 0; i < descCount; ++i) {
            globalMin = glm::min(globalMin, nodeMins[i]);
            globalMax = glm::max(globalMax, nodeMaxs[i]);
        }

        cookedModel.globalCollider.aabb.min = globalMin;
        cookedModel.globalCollider.aabb.max = globalMax;

        glm::vec3 globalCenter = (globalMin + globalMax) * 0.5f;
        cookedModel.globalCollider.sphere.center = globalCenter;

        // SPHERE MAP FÁZIS
        std::vector<float> nodeMaxRadiusSq(descCount, 0.0f);

        taskflow.for_each_index(size_t(0), descCount, size_t(1), 
            [&](size_t i) {
            const auto& desc = cookedModel.meshNodeDescriptors[i];
            if (desc.meshIndex >= cookedModel.meshes.size() || desc.nodeIndex >= cookedModel.nodeTransforms.size())
                return;

            const CookedMesh& mesh = cookedModel.meshes[desc.meshIndex];
            const glm::mat4& transform = cookedModel.nodeTransforms[desc.nodeIndex].globalTransform;

            float maxRadiusSq = 0.0f;
            for (const auto& v : mesh.vertices) {
                glm::vec3 worldPos = glm::vec3(transform * glm::vec4(v.position, 1.0f));
                float distSq = glm::length2(worldPos - globalCenter);
                if (distSq > maxRadiusSq)
                    maxRadiusSq = distSq;
            }
            nodeMaxRadiusSq[i] = maxRadiusSq;
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();

        // SPHERE REDUCE
        float finalMaxRadiusSq = 0.0f;
        for (float rSq : nodeMaxRadiusSq) {
            if (rSq > finalMaxRadiusSq) finalMaxRadiusSq = rSq;
        }
        cookedModel.globalCollider.sphere.radius = std::sqrt(finalMaxRadiusSq);
    }
}