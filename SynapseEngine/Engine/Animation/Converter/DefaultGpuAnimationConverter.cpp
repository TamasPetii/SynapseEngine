#include "DefaultGpuAnimationConverter.h"
#include "Engine/Mesh/Utils/MeshUtils.h"

namespace Syn
{
    constexpr uint32_t MAX_LODS = 4;

    GpuBatchedAnimation DefaultGpuAnimationConverter::Convert(const CookedAnimation& cookedAnimation, const CookedModel& baseModel) const
    {
        GpuBatchedAnimation result;

        result.descriptor.frameCount = cookedAnimation.frameCount;
        result.descriptor.nodeCount = cookedAnimation.nodeCount;
        result.descriptor.durationInSeconds = cookedAnimation.durationInSeconds;
        result.descriptor.sampleRate = cookedAnimation.sampleRate;

        for (size_t instanceIdx = 0; instanceIdx < baseModel.meshNodeDescriptors.size(); ++instanceIdx)
        {
            const auto& instanceDesc = baseModel.meshNodeDescriptors[instanceIdx];
            const auto& skinData = cookedAnimation.meshSkins[instanceDesc.meshIndex];

            for (const auto& vSkin : skinData.vertices)
            {
                result.vertexSkinData.push_back(vSkin);
            }
        }

        uint32_t totalMeshesPerFrame = 0;
        uint32_t totalMeshletsPerFrame = 0;

        for (uint32_t f = 0; f < cookedAnimation.frameCount; ++f)
        {
            const CookedAnimationFrame& cookedFrame = cookedAnimation.frames[f];

            for (const auto& transform : cookedFrame.bakedNodeTransforms)
            {
                result.nodeTransforms.push_back(transform);
            }

            GpuMeshCollider globalCol{};
            globalCol.center = cookedFrame.globalCollider.sphere.center;
            globalCol.radius = cookedFrame.globalCollider.sphere.radius;
            globalCol.aabbMin = cookedFrame.globalCollider.aabb.min;
            globalCol.aabbMax = cookedFrame.globalCollider.aabb.max;
            result.frameGlobalColliders.push_back(globalCol);

            bool isFirstFrame = (f == 0);

            for (size_t instanceIdx = 0; instanceIdx < baseModel.meshNodeDescriptors.size(); ++instanceIdx)
            {
                if (isFirstFrame) totalMeshesPerFrame++;

                const auto& instanceDesc = baseModel.meshNodeDescriptors[instanceIdx];
                const CookedAnimationFrameMesh& animMesh = cookedFrame.meshes[instanceDesc.meshIndex];
                const CookedMesh& cookedMesh = baseModel.meshes[instanceDesc.meshIndex];

                const glm::mat4& nodeTransform = baseModel.nodeTransforms[instanceDesc.nodeIndex].globalTransform;
                const glm::mat4& nodeTransformIT = baseModel.nodeTransforms[instanceDesc.nodeIndex].globalTransformIT;

                GpuMeshCollider localMeshCollider{};
                localMeshCollider.center = animMesh.collider.sphere.center;
                localMeshCollider.radius = animMesh.collider.sphere.radius;
                localMeshCollider.aabbMin = animMesh.collider.aabb.min;
                localMeshCollider.aabbMax = animMesh.collider.aabb.max;

                GpuMeshCollider modelSpaceCollider = MeshUtils::TransformCollider(localMeshCollider, nodeTransform);
                result.frameMeshColliders.push_back(modelSpaceCollider);

                for (uint32_t lodLevel = 0; lodLevel < MAX_LODS; ++lodLevel)
                {
                    if (lodLevel < animMesh.lods.size())
                    {
                        const auto& lodData = animMesh.lods[lodLevel];

                        for (const auto& meshlet : lodData.meshlets)
                        {
                            if (isFirstFrame) totalMeshletsPerFrame++;

                            GpuMeshletCollider colliderDesc{};
                            colliderDesc.center = meshlet.collider.sphere.center;
                            colliderDesc.radius = meshlet.collider.sphere.radius;
                            colliderDesc.aabbMin = meshlet.collider.aabb.min;
                            colliderDesc.aabbMax = meshlet.collider.aabb.max;
                            colliderDesc.apex = meshlet.collider.cone.apex;
                            colliderDesc.axis = meshlet.collider.cone.axis;
                            colliderDesc.cutoff = meshlet.collider.cone.cutoff;

                            colliderDesc = MeshUtils::TransformCollider(colliderDesc, nodeTransform, nodeTransformIT);
                            result.frameMeshletColliders.push_back(colliderDesc);
                        }
                    }
                }
            }
        }

        result.descriptor.globalVertexCount = static_cast<uint32_t>(result.vertexSkinData.size());
        result.descriptor.globalMeshCount = totalMeshesPerFrame;
        result.descriptor.globalMeshletCount = totalMeshletsPerFrame;

        return result;
    }
}