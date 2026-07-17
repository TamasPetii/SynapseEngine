#include "AnimationColliderProcessor.h"
#include "Engine/ServiceLocator.h"
#include <limits>

#include <meshoptimizer.h>
#include <chrono>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Syn
{
    void AnimationColliderProcessor::Process(CookedAnimation& inOutAnimation, const CpuModelData& baseModel)
    {
        tf::Taskflow taskflow;

        for (uint32_t frameIndex = 0; frameIndex < inOutAnimation.frameCount; ++frameIndex)
        {
            taskflow.emplace([&, frameIndex](tf::Subflow& subflow) {
                ComputeFrameColliders(frameIndex, inOutAnimation, baseModel, subflow);
                });
        }

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();

		ComputeGlobalAnimationCollider(inOutAnimation);
    }

    void AnimationColliderProcessor::ComputeGlobalAnimationCollider(CookedAnimation& anim)
    {
        glm::vec3 animMin(std::numeric_limits<float>::max());
        glm::vec3 animMax(std::numeric_limits<float>::lowest());

        for (uint32_t f = 0; f < anim.frameCount; ++f)
        {
            const auto& frameCollider = anim.frames[f].globalCollider;
            animMin = glm::min(animMin, frameCollider.aabb.min);
            animMax = glm::max(animMax, frameCollider.aabb.max);
        }

        anim.globalFrameCollider.aabb.min = animMin;
        anim.globalFrameCollider.aabb.max = animMax;
        anim.globalFrameCollider.sphere.center = (animMin + animMax) * 0.5f;

        float maxRadius = 0.0f;
        for (uint32_t f = 0; f < anim.frameCount; ++f)
        {
            const auto& frameCollider = anim.frames[f].globalCollider;

            float dist = glm::length(frameCollider.sphere.center - anim.globalFrameCollider.sphere.center);
            float requiredRadius = dist + frameCollider.sphere.radius;

            if (requiredRadius > maxRadius)
                maxRadius = requiredRadius;
        }

        anim.globalFrameCollider.sphere.radius = maxRadius;
    }

    void AnimationColliderProcessor::ComputeFrameColliders(uint32_t frameIndex, CookedAnimation& anim, const CpuModelData& model, tf::Subflow& subflow)
    {
        CookedAnimationFrame& currentFrame = anim.frames[frameIndex];
        currentFrame.meshes.resize(model.globalMeshCount);

        std::vector<glm::vec3> meshMins(model.globalMeshCount, glm::vec3(std::numeric_limits<float>::max()));
        std::vector<glm::vec3> meshMaxs(model.globalMeshCount, glm::vec3(std::numeric_limits<float>::lowest()));
        std::vector<glm::vec3> meshCenters(model.globalMeshCount);
        std::vector<float> meshRadii(model.globalMeshCount);
        std::vector<glm::vec3> deformedPositions(model.globalVertexCount);

        tf::GuidedPartitioner partitioner(1);

        subflow.for_each_index(size_t(0), model.globalMeshCount, size_t(1),
            [&](size_t m) {
                //Todo: Change to new cpu model!
                const auto& meshDesc = model.meshDescriptors[m * 4];
                uint32_t vOffset = meshDesc.vertexOffset;
                uint32_t vCount = meshDesc.vertexCount;

                const CookedAnimationMeshSkin& skinData = anim.meshSkins[m];
                CookedAnimationFrameMesh& frameMesh = currentFrame.meshes[m];

                glm::vec3 mMin(std::numeric_limits<float>::max());
                glm::vec3 mMax(std::numeric_limits<float>::lowest());

                for (size_t v = 0; v < vCount; ++v)
                {
                    glm::mat4 skinMat(0.0f);

                    for (int i = 0; i < 4; ++i)
                    {
                        uint32_t boneIdx = skinData.vertices[v].boneIndices[i];
                        float weight = skinData.vertices[v].boneWeights[i];

                        if (boneIdx != UINT32_MAX && weight > 0.0f)
                        {
                            skinMat += currentFrame.bakedNodeTransforms[boneIdx].globalTransform * weight;
                        }
                    }

                    glm::vec3 originalPos = model.vertices[vOffset + v];
                    glm::vec3 animPos = glm::vec3(skinMat * glm::vec4(originalPos, 1.0f));
                    deformedPositions[vOffset + v] = animPos;

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

                uint32_t lodCount = 4;
                frameMesh.lods.resize(lodCount);

                if (true /*model.meshletVertexIndices.has_value() && model.meshletTriangleIndices.has_value() && model.meshletDescriptors.has_value()*/)
                {
                    const auto& rawVerts = model.meshletVertexIndices;
                    const auto& rawTris = model.meshletTriangleIndices;
                    const auto& meshletDescs = model.meshletDescriptors;

                    for (size_t l = 0; l < lodCount; ++l)
                    {
                        uint32_t lodDescIndex = static_cast<uint32_t>((static_cast<uint32_t>(m) * 4) + l);
                        if (lodDescIndex >= static_cast<uint32_t>(model.meshletDrawDescriptors.size())) continue;

                        const auto& drawDesc = model.meshletDrawDescriptors[lodDescIndex];
                        uint32_t meshletOffset = drawDesc.meshletOffset;
                        uint32_t meshletCount = drawDesc.meshletCount;

                        CookedAnimationFrameLod& frameLod = frameMesh.lods[l];
                        frameLod.meshlets.resize(meshletCount);

                        for (size_t ml = 0; ml < meshletCount; ++ml)
                        {
                            uint32_t globalMeshletIdx = static_cast<uint32_t>(meshletOffset + ml);
                            const auto& meshletDesc = meshletDescs[globalMeshletIdx];
                            CookedAnimationFrameMeshlet& frameMeshlet = frameLod.meshlets[ml];

                            const uint32_t* mVertices = &rawVerts[meshletDesc.vertexIndicesOffset];
                            const uint8_t* mTriangles = &rawTris[meshletDesc.triangleIndicesOffset];

                            meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                                mVertices,
                                mTriangles,
                                meshletDesc.triangleCount,
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

                            for (uint32_t i = 0; i < meshletDesc.vertexCount; ++i) {
                                uint32_t globalVIdx = mVertices[i];
                                mlMin = glm::min(mlMin, deformedPositions[globalVIdx]);
                                mlMax = glm::max(mlMax, deformedPositions[globalVIdx]);
                            }

                            frameMeshlet.collider.aabb.min = mlMin;
                            frameMeshlet.collider.aabb.max = mlMax;
                        }
                    }
                }    
            },
            partitioner
        );

        subflow.join();

        glm::vec3 globalMin(std::numeric_limits<float>::max());
        glm::vec3 globalMax(std::numeric_limits<float>::lowest());

        for (size_t i = 0; i < model.globalMeshCount; ++i) {
            globalMin = glm::min(globalMin, meshMins[i]);
            globalMax = glm::max(globalMax, meshMaxs[i]);
        }

        currentFrame.globalCollider.aabb.min = globalMin;
        currentFrame.globalCollider.aabb.max = globalMax;
        currentFrame.globalCollider.sphere.center = (globalMin + globalMax) * 0.5f;

        float globalMaxRadius = 0.0f;
        for (size_t i = 0; i < model.globalMeshCount; ++i) {
            float dist = glm::length(meshCenters[i] - currentFrame.globalCollider.sphere.center);
            float boundsRadius = dist + meshRadii[i];
            if (boundsRadius > globalMaxRadius) {
                globalMaxRadius = boundsRadius;
            }
        }

        currentFrame.globalCollider.sphere.radius = globalMaxRadius;
    }
}