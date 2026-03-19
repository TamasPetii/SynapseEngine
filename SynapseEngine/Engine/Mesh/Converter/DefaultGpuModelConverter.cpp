#include "DefaultGpuModelConverter.h"
#include <algorithm>
#include "Engine/Mesh/Utils/MeshUtils.h"
#include "Engine/Material/MaterialNames.h"

namespace Syn
{
    constexpr uint32_t MAX_LODS = 4;
    constexpr float GLOBAL_LOD_DISTANCES[MAX_LODS] = { 10.0f, 25.0f, 50.0f, 100.0f };

    GpuBatchedModel DefaultGpuModelConverter::Convert(const CookedModel& cookedModel) const
    {
        GpuBatchedModel result;
        result.globalCollider.center = cookedModel.globalCollider.sphere.center;
        result.globalCollider.radius = cookedModel.globalCollider.sphere.radius;
        result.globalCollider.aabbMin = cookedModel.globalCollider.aabb.min;
        result.globalCollider.aabbMax = cookedModel.globalCollider.aabb.max;
		result.materials = cookedModel.materials;

        if (result.materials.empty()) {
            MaterialInfo defaultRef{};
            defaultRef.name = MaterialNames::EngineDefault;
            result.materials.push_back(defaultRef);
        }

        const uint32_t globalMeshIdOffset = static_cast<uint32_t>(result.indexedData.meshDescriptors.size()) / MAX_LODS;
        const uint32_t globalVertexOffset = static_cast<uint32_t>(result.vertexData.vertexPositions.size());
        const uint32_t globalNodeOffset = static_cast<uint32_t>(result.nodeTransforms.size());

        for (const auto& node : cookedModel.nodeTransforms) {
            GpuNodeTransform gpuNode{};
            gpuNode.transform = node.globalTransform;
            gpuNode.transformIT = node.globalTransformIT;
            result.nodeTransforms.push_back(gpuNode);
        }

        if (result.indexedData.lodDescriptors.empty()) {
            for (uint32_t i = 0; i < MAX_LODS; ++i) {
                GpuMeshLodDescriptor desc{};
                desc.distanceThreshold = GLOBAL_LOD_DISTANCES[i];
                result.indexedData.lodDescriptors.push_back(desc);
            }
        }

        for (size_t instanceIdx = 0; instanceIdx < cookedModel.meshNodeDescriptors.size(); ++instanceIdx)
        {
            const auto& instanceDesc = cookedModel.meshNodeDescriptors[instanceIdx];

            const uint32_t globalMeshID = globalMeshIdOffset + static_cast<uint32_t>(instanceIdx);
            const auto& cookedMesh = cookedModel.meshes[instanceDesc.meshIndex];
            uint32_t globalNodeIndex = globalNodeOffset + instanceDesc.nodeIndex;
            const uint32_t currentMeshVertexOffset = static_cast<uint32_t>(result.vertexData.vertexPositions.size());

            // MESH COLLIDER BEMÁSOLÁSA
            GpuMeshCollider localMeshCollider{};
            localMeshCollider.center = cookedMesh.collider.sphere.center;
            localMeshCollider.radius = cookedMesh.collider.sphere.radius;
            localMeshCollider.aabbMin = cookedMesh.collider.aabb.min;
            localMeshCollider.aabbMax = cookedMesh.collider.aabb.max;

            const glm::mat4& nodeTransform = cookedModel.nodeTransforms[instanceDesc.nodeIndex].globalTransform;
            const glm::mat4& nodeTransformIT = cookedModel.nodeTransforms[instanceDesc.nodeIndex].globalTransformIT;

            GpuMeshCollider modelSpaceCollider = MeshUtils::TransformCollider(localMeshCollider, nodeTransform);
            result.indexedData.meshColliders.push_back(modelSpaceCollider);

            // VERTEX ADATOK MÁSOLÁSA ÉS INDEXEK ELTOLÁSA
            for (const auto& v : cookedMesh.vertices)
            {
                GpuVertexPosition pos;
                pos.position = v.position;
                pos.SetMeshAndNodeIndex(static_cast<uint16_t>(globalMeshID), static_cast<uint16_t>(globalNodeIndex));
                result.vertexData.vertexPositions.push_back(pos);

                GpuVertexAttributes attr;
                attr.normal = v.normal;
                attr.tangent = v.tangent;
                attr.uv_x = v.uv.x;
                attr.uv_y = v.uv.y;
                result.vertexData.vertexAttributes.push_back(attr);
            }

            GpuMeshDescriptor lastValidMeshDesc{};
            GpuMeshletDrawDescriptor lastValidMeshletDrawDesc{};

            for (uint32_t lodLevel = 0; lodLevel < MAX_LODS; ++lodLevel)
            {
                if (lodLevel < cookedMesh.lods.size())
                {
                    const auto& lodData = cookedMesh.lods[lodLevel];

                    { //Normal
                        GpuMeshDescriptor meshDesc{};
                        meshDesc.vertexOffset = currentMeshVertexOffset;
                        meshDesc.vertexCount = static_cast<uint32_t>(cookedMesh.vertices.size());
                        meshDesc.indexOffset = static_cast<uint32_t>(result.indexedData.indices.size());
                        meshDesc.indexCount = static_cast<uint32_t>(lodData.indices.size());
                        meshDesc.materialIndex = cookedMesh.materialIndex;

                        result.indexedData.meshDescriptors.push_back(meshDesc);
                        lastValidMeshDesc = meshDesc;

                        for (uint32_t idx : lodData.indices) {
                            result.indexedData.indices.push_back(idx + currentMeshVertexOffset);
                        }
                    }

                    { //Mesh Shader
                        GpuMeshletDrawDescriptor meshletDrawDesc{};
                        meshletDrawDesc.meshletOffset = static_cast<uint32_t>(result.meshletData.meshletDescriptors.size());
                        meshletDrawDesc.meshletCount = static_cast<uint32_t>(lodData.meshlets.size());
                        meshletDrawDesc.materialIndex = cookedMesh.materialIndex;

                        result.meshletData.drawDescriptors.push_back(meshletDrawDesc);
                        lastValidMeshletDrawDesc = meshletDrawDesc;

                        // Apró Meshletek Adatainak Másolása
                        for (const auto& cookedMeshlet : lodData.meshlets)
                        {
                            GpuMeshletDescriptor meshletDesc{};
                            meshletDesc.vertexIndicesOffset = static_cast<uint32_t>(result.meshletData.vertexIndices.size());
                            meshletDesc.vertexCount = static_cast<uint8_t>(cookedMeshlet.vertexCount);
                            meshletDesc.triangleIndicesOffset = static_cast<uint32_t>(result.meshletData.triangleIndices.size());
                            meshletDesc.triangleCount = static_cast<uint8_t>(cookedMeshlet.triangleCount);
                            result.meshletData.meshletDescriptors.push_back(meshletDesc);

                            GpuMeshletCollider colliderDesc{};
                            colliderDesc.center = cookedMeshlet.collider.sphere.center;
                            colliderDesc.radius = cookedMeshlet.collider.sphere.radius;
                            colliderDesc.aabbMax = cookedMeshlet.collider.aabb.max;
                            colliderDesc.aabbMin = cookedMeshlet.collider.aabb.min;
                            colliderDesc.axis = cookedMeshlet.collider.cone.axis;
                            colliderDesc.cutoff = cookedMeshlet.collider.cone.cutoff;
                            colliderDesc.apex = cookedMeshlet.collider.cone.apex;

                            colliderDesc = MeshUtils::TransformCollider(colliderDesc, nodeTransform, nodeTransformIT);
                            result.meshletData.meshletColliders.push_back(colliderDesc);

                            uint32_t localVtxOffset = cookedMeshlet.vertexOffset;
                            for (uint32_t i = 0; i < cookedMeshlet.vertexCount; ++i) {
                                uint32_t globalVertexIdx = lodData.meshletVertexIndices[localVtxOffset + i] + currentMeshVertexOffset;
                                result.meshletData.vertexIndices.push_back(globalVertexIdx);
                            }

                            uint32_t localTriOffset = cookedMeshlet.triangleOffset;
                            for (uint32_t i = 0; i < (cookedMeshlet.triangleCount * 3); ++i) {
                                result.meshletData.triangleIndices.push_back(lodData.meshletTriangleIndices[localTriOffset + i]);
                            }
                        }
                    }         
                }
                else
                {
                    result.indexedData.meshDescriptors.push_back(lastValidMeshDesc);
                    result.meshletData.drawDescriptors.push_back(lastValidMeshletDrawDesc);
                }
            }

            for (uint32_t i = 0; i < MAX_LODS; ++i) {
                result.indexedData.lodDescriptors[i].meshCount++;
                result.indexedData.lodDescriptors[i].indexCount = static_cast<uint32_t>(result.indexedData.indices.size());
            }
        }

        result.globalVertexCount = static_cast<uint32_t>(result.vertexData.vertexPositions.size());
        result.globalIndexCount = static_cast<uint32_t>(result.indexedData.indices.size());
        result.globalAverageLodIndexCount = result.globalIndexCount / MAX_LODS;
        result.globalMeshCount = static_cast<uint32_t>(result.indexedData.meshDescriptors.size() / 4);

        return result;
    }
}