#include "AnimationColliderProcessor.h"
#include "Engine/ServiceLocator.h"
#include <limits>

#include <meshoptimizer.h>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Syn
{
    void AnimationColliderProcessor::Process(CookedAnimation& inOutAnimation, const CookedModel& baseModel)
    {
        tf::Taskflow taskflow;

        for (uint32_t frameIndex = 0; frameIndex < inOutAnimation.frameCount; ++frameIndex)
        {
            taskflow.emplace([&, frameIndex](tf::Subflow& subflow) {
                ComputeFrameColliders(frameIndex, inOutAnimation, baseModel, subflow);
                });
        }

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
    }

    void AnimationColliderProcessor::ComputeFrameColliders(uint32_t frameIndex, CookedAnimation& anim, const CookedModel& model, tf::Subflow& subflow)
    {
        CookedAnimationFrame& currentFrame = anim.frames[frameIndex];
        currentFrame.meshes.resize(model.meshes.size());

        std::vector<glm::vec3> meshMins(model.meshes.size(), glm::vec3(std::numeric_limits<float>::max()));
        std::vector<glm::vec3> meshMaxs(model.meshes.size(), glm::vec3(std::numeric_limits<float>::lowest()));
        std::vector<glm::vec3> meshCenters(model.meshes.size());
        std::vector<float> meshRadii(model.meshes.size());

        tf::GuidedPartitioner partitioner(1);

        subflow.for_each_index(size_t(0), model.meshes.size(), size_t(1),
            [&](size_t m) {
                const CookedMesh& staticMesh = model.meshes[m];
                const CookedAnimationMeshSkin& skinData = anim.meshSkins[m];
                CookedAnimationFrameMesh& frameMesh = currentFrame.meshes[m];

                std::vector<glm::vec3> deformedPositions(staticMesh.vertices.size());
                glm::vec3 mMin(std::numeric_limits<float>::max());
                glm::vec3 mMax(std::numeric_limits<float>::lowest());

                for (size_t v = 0; v < staticMesh.vertices.size(); ++v)
                {
                    glm::mat4 skinMat(0.0f);
                    for (int i = 0; i < 4; ++i)
                    {
                        uint32_t boneIdx = skinData.vertices[v].boneIndices[i];
                        float weight = skinData.vertices[v].boneWeights[i];

                        if (boneIdx != UINT32_MAX && weight > 0.0f)
                        {
                            skinMat += currentFrame.bakedNodeTransforms[boneIdx] * weight;
                        }
                    }

                    if (skinMat == glm::mat4(0.0f)) 
                        skinMat = glm::mat4(1.0f);

                    glm::vec3 animPos = glm::vec3(skinMat * glm::vec4(staticMesh.vertices[v].position, 1.0f));
                    deformedPositions[v] = animPos;

                    mMin = glm::min(mMin, animPos);
                    mMax = glm::max(mMax, animPos);
                }

                frameMesh.collider.aabb.min = mMin;
                frameMesh.collider.aabb.max = mMax;
                frameMesh.collider.sphere.center = (mMin + mMax) * 0.5f;

                float maxRadiusSq = 0.0f;
                for (const auto& pos : deformedPositions) {
                    float distSq = glm::length2(pos - frameMesh.collider.sphere.center);
                    if (distSq > maxRadiusSq) maxRadiusSq = distSq;
                }
                frameMesh.collider.sphere.radius = std::sqrt(maxRadiusSq);

                meshMins[m] = mMin;
                meshMaxs[m] = mMax;
                meshCenters[m] = frameMesh.collider.sphere.center;
                meshRadii[m] = frameMesh.collider.sphere.radius;

                frameMesh.lods.resize(staticMesh.lods.size());
                for (size_t l = 0; l < staticMesh.lods.size(); ++l)
                {
                    const CookedMeshLod& staticLod = staticMesh.lods[l];
                    CookedAnimationFrameLod& frameLod = frameMesh.lods[l];
                    frameLod.meshlets.resize(staticLod.meshlets.size());

                    for (size_t ml = 0; ml < staticLod.meshlets.size(); ++ml)
                    {
                        const CookedMeshlet& staticMeshlet = staticLod.meshlets[ml];
                        CookedAnimationFrameMeshlet& frameMeshlet = frameLod.meshlets[ml];

                        meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                            &staticLod.meshletVertexIndices[staticMeshlet.vertexOffset],
                            &staticLod.meshletTriangleIndices[staticMeshlet.triangleOffset],
                            staticMeshlet.triangleCount,
                            &deformedPositions[0].x,
                            deformedPositions.size(),
                            sizeof(glm::vec3)
                        );

                        frameMeshlet.collider.sphere.center = glm::vec3(bounds.center[0], bounds.center[1], bounds.center[2]);
                        frameMeshlet.collider.sphere.radius = bounds.radius;
                        frameMeshlet.collider.cone.apex = glm::vec3(bounds.cone_apex[0], bounds.cone_apex[1], bounds.cone_apex[2]);
                        frameMeshlet.collider.cone.axis = glm::vec3(bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2]);
                        frameMeshlet.collider.cone.cutoff = bounds.cone_cutoff;

                        glm::vec3 mlMin(std::numeric_limits<float>::max());
                        glm::vec3 mlMax(std::numeric_limits<float>::lowest());
                        for (uint32_t i = 0; i < staticMeshlet.vertexCount; ++i) {
                            uint32_t vIdx = staticLod.meshletVertexIndices[staticMeshlet.vertexOffset + i];
                            mlMin = glm::min(mlMin, deformedPositions[vIdx]);
                            mlMax = glm::max(mlMax, deformedPositions[vIdx]);
                        }
                        frameMeshlet.collider.aabb.min = mlMin;
                        frameMeshlet.collider.aabb.max = mlMax;
                    }
                }
            },
            partitioner
        );

        subflow.join();

        glm::vec3 globalMin(std::numeric_limits<float>::max());
        glm::vec3 globalMax(std::numeric_limits<float>::lowest());

        for (size_t i = 0; i < model.meshes.size(); ++i) {
            globalMin = glm::min(globalMin, meshMins[i]);
            globalMax = glm::max(globalMax, meshMaxs[i]);
        }

        currentFrame.globalCollider.aabb.min = globalMin;
        currentFrame.globalCollider.aabb.max = globalMax;
        currentFrame.globalCollider.sphere.center = (globalMin + globalMax) * 0.5f;

        float globalMaxRadius = 0.0f;
        for (size_t i = 0; i < model.meshes.size(); ++i) {
            float dist = glm::length(meshCenters[i] - currentFrame.globalCollider.sphere.center);
            float boundsRadius = dist + meshRadii[i];
            if (boundsRadius > globalMaxRadius) {
                globalMaxRadius = boundsRadius;
            }
        }

        currentFrame.globalCollider.sphere.radius = globalMaxRadius;
    }
}