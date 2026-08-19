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

#include "PointLightShadowCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Logger/SynLog.h"
#include "PointLightShadowRenderSystem.h"
#include "PointLightCullingSystem.h"
#include "PointLightShadowAtlasSystem.h"
#include "PointLightSystem.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "Engine/System/Rendering/AnimationSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"
#include "Engine/System/Core/StaticSpatialSahSystem.h"

#include "Engine/Component/Rendering/PipelineOverrideComponent.h"
#include "Engine/System/Rendering/PipelineOverrideSystem.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include <atomic>
#include <algorithm>

#include "Engine/Component/Core/TagComponent.h"
#include "Engine/System/Core/TagSystem.h"

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    struct EntityCullData
    {
        EntityID entity;
        const glm::mat4& transform;
        GpuMeshCollider globalWorldCollider;
        uint32_t meshCount;
        const StaticMesh* modelResource;
        const ModelAllocationInfo* modelAlloc;
        bool hasAnimation;
        uint32_t animFrameIndex;
        const Animation* animResource;
        std::span<const uint32_t> materialOverrides;
        std::span<const uint32_t> pipelineOverrides;
    };

    std::vector<TypeID> PointLightShadowCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<PointLightShadowRenderSystem>::ID,
            TypeInfo<PointLightCullingSystem>::ID,
            TypeInfo<PointLightShadowAtlasSystem>::ID,
            TypeInfo<PointLightSystem>::ID,
            TypeInfo<TransformSystem>::ID,
            TypeInfo<ModelSystem>::ID,
            TypeInfo<RenderSystem>::ID,
            TypeInfo<AnimationSystem>::ID,
            TypeInfo<MaterialSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<StaticSpatialSahSystem>::ID,
            TypeInfo<TagSystem>::ID,
            TypeInfo<PipelineOverrideSystem>::ID
        };
    }

    void PointLightShadowCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();

        tf::Task initTask = this->EmplaceTask(subflow, "Point Shadow Update Init", [this, drawData]() {
            auto& shadowGroup = drawData->PointLightShadow;
            auto& mainGroup = drawData->Models;

            shadowGroup.appendedInstanceCount.store(0, std::memory_order_relaxed);

            for (uint32_t i = 0; i < mainGroup.activeTraditionalCount; ++i) {
                shadowGroup.traditionalCmds[i].instanceCount = 0;
            }
            for (uint32_t i = 0; i < mainGroup.activeMeshletCount; ++i) {
                shadowGroup.meshletCmds[i].groupCountX = 0;
            }

            uint32_t expectedMaxInstances = mainGroup.totalAllocatedInstances * POINT_SHADOW_MULTIPLIER;
            if (_sortBuffer.size() < expectedMaxInstances) {
                _sortBuffer.resize(expectedMaxInstances);
            }
            });

        if (settings->culling.pointLightCullingDevice == CullingDeviceType::GPU)
            return;

        if (settings->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU)
            return;

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto animPool = registry->GetPool<AnimationComponent>();
        auto overridePool = registry->GetPool<MaterialOverrideComponent>();
        auto pipeOverridePool = registry->GetPool<PipelineOverrideComponent>();
        auto shadowPool = registry->GetPool<PointLightShadowComponent>();
        auto lightPool = registry->GetPool<PointLightComponent>();
        auto tagPool = registry->GetPool<TagComponent>();

        EntityID cameraEntity = scene->GetSceneCameraEntity();
        if (!modelPool || !transformPool || !cameraPool || cameraEntity == NULL_ENTITY || !shadowPool || !lightPool) return;

        uint32_t activeShadowLightCount = drawData->PointLightShadow.visibleLightCount;
        if (activeShadowLightCount == 0) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto animationManager = ServiceLocator::Get<AnimationManager>();
        auto materialManager = ServiceLocator::Get<MaterialManager>();

        auto& modelSnapshot = scene->GetSystemContext().modelSnapshots;
        auto& animSnapshot = scene->GetSystemContext().animationSnapshots;
        auto& matTypeSnapshot = scene->GetSystemContext().materialRenderTypes;

        // Extract Entity Data (Runs once per entity)
        auto withEntityData = [modelPool, transformPool, animPool, overridePool, pipeOverridePool, modelSnapshot, tagPool, animSnapshot, drawData]
        (EntityID entity, auto&& nextFunc) {
            if (!modelPool->Has(entity)) return;

            const auto& modelComp = modelPool->Get(entity);
            if (modelComp.modelIndex == NULL_INDEX || modelComp.modelIndex >= drawData->Models.modelAllocations.Size()) return;

            if (tagPool && tagPool->Has(entity)) {
                const auto& tag = tagPool->Get(entity);
                if (!tag.globalEnabled || !modelComp.castShadow) {
                    return;
                }
            }

            const auto& snapshotEntry = modelSnapshot[modelComp.modelIndex];
            if (snapshotEntry.resource == nullptr || snapshotEntry.state != ResourceState::Ready) return;

            const auto& transformComp = transformPool->Get(entity);
            const auto& modelAlloc = drawData->Models.modelAllocations[modelComp.modelIndex];

            bool hasAnimation = false;
            uint32_t animFrameIndex = 0;
            const Animation* animResource = nullptr;

            if (animPool && animPool->Has(entity)) {
                const auto& animComp = animPool->Get(entity);
                if (animComp.isReady && animComp.animationIndex != NULL_INDEX && animComp.animationIndex < animSnapshot.size()) {
                    const auto& aSnapshotEntry = animSnapshot[animComp.animationIndex];
                    if (aSnapshotEntry.resource != nullptr && aSnapshotEntry.state == ResourceState::Ready) {
                        hasAnimation = true;
                        animFrameIndex = animComp.frameIndex;
                        animResource = static_cast<const Animation*>(aSnapshotEntry.resource.get());
                    }
                }
            }

            std::span<const uint32_t> overrides;
            if (overridePool && overridePool->Has(entity)) {
                overrides = overridePool->Get(entity).materials;
            }

            std::span<const uint32_t> pipeOverrides;
            if (pipeOverridePool && pipeOverridePool->Has(entity)) {
                pipeOverrides = pipeOverridePool->Get(entity).pipelines;
            }

            const StaticMesh* modelResource = static_cast<const StaticMesh*>(snapshotEntry.resource.get());

            GpuMeshCollider globalLocalCollider = hasAnimation ?
                animResource->cpuData.frameGlobalColliders[animFrameIndex] : modelResource->cpuData.globalCollider;

            GpuMeshCollider worldCollider = MeshUtils::TransformCollider(globalLocalCollider, transformComp.transform);

            EntityCullData data{
                .entity = entity, 
                .transform = transformComp.transform, 
                .globalWorldCollider = worldCollider,
                .meshCount = modelAlloc.meshAllocationCount / 4, 
                .modelResource = modelResource,
                .modelAlloc = &modelAlloc, 
                .hasAnimation = hasAnimation, 
                .animFrameIndex = animFrameIndex,
                .animResource = animResource, 
                .materialOverrides = overrides,
                .pipelineOverrides = pipeOverrides
            };
            nextFunc(data);
            };

        // Mesh Level Culling & Face Intersection Logic
        auto cullMeshes = [this, settings, drawData, matTypeSnapshot, cameraComp, screenRes]
        (const EntityCullData& data, uint32_t lightIndex, const glm::vec3& lightPos, float lightRadius, bool parentFullyInside) {

            for (uint32_t m = 0; m < data.meshCount; ++m)
            {
                uint32_t matIdx = data.modelResource->cpuData.meshMaterialIndices[m];
                if (!data.materialOverrides.empty() && m < data.materialOverrides.size() && data.materialOverrides[m] != UINT32_MAX)
                    matIdx = data.materialOverrides[m];

                MaterialRenderType matType = (matIdx < matTypeSnapshot.size()) ? matTypeSnapshot[matIdx] : MaterialRenderType::Opaque1Sided;

                uint32_t pipeIdx = static_cast<uint32_t>(data.modelResource->cpuData.baseDrawCommands[m * 4].pipelineRenderType);
                if (!data.pipelineOverrides.empty() && m < data.pipelineOverrides.size() && data.pipelineOverrides[m] != UINT32_MAX) {
                    pipeIdx = data.pipelineOverrides[m];
                }

                GpuMeshCollider worldCollider;
                if (data.meshCount > 1) {
                    GpuMeshCollider localCollider = data.hasAnimation ?
                        data.animResource->cpuData.frameMeshColliders[(data.animFrameIndex * data.animResource->cpuData.descriptor.globalMeshCount) + m] :
                        data.modelResource->cpuData.meshColliders[m];

                    worldCollider = MeshUtils::TransformCollider(localCollider, data.transform);

                    if (!parentFullyInside && settings->culling.enableFrustumCulling && settings->culling.enableMeshFrustumCulling) {
                        if (!CollisionTester::IsInSphere(worldCollider, lightPos, lightRadius)) {
                            continue;
                        }
                    }
                }
                else {
                    worldCollider = data.globalWorldCollider;
                }

                // Fast spatial 6-way AABB face overlap detection based on axes
                glm::vec3 relMin = (worldCollider.center - glm::vec3(worldCollider.radius)) - lightPos;
                glm::vec3 relMax = (worldCollider.center + glm::vec3(worldCollider.radius)) - lightPos;

                auto getMinAbs = [](float minVal, float maxVal) {
                    if (minVal <= 0.0f && maxVal >= 0.0f) return 0.0f;
                    return std::min(std::abs(minVal), std::abs(maxVal));
                    };

                float minAbsX = getMinAbs(relMin.x, relMax.x);
                float minAbsY = getMinAbs(relMin.y, relMax.y);
                float minAbsZ = getMinAbs(relMin.z, relMax.z);

                bool faceVis[6];
                faceVis[0] = relMax.x > 0.0f && relMax.x >= minAbsY && relMax.x >= minAbsZ; // +X
                faceVis[1] = relMin.x < 0.0f && -relMin.x >= minAbsY && -relMin.x >= minAbsZ; // -X
                faceVis[2] = relMax.y > 0.0f && relMax.y >= minAbsX && relMax.y >= minAbsZ; // +Y
                faceVis[3] = relMin.y < 0.0f && -relMin.y >= minAbsX && -relMin.y >= minAbsZ; // -Y
                faceVis[4] = relMax.z > 0.0f && relMax.z >= minAbsX && relMax.z >= minAbsY; // +Z
                faceVis[5] = relMin.z < 0.0f && -relMin.z >= minAbsX && -relMin.z >= minAbsY; // -Z

                // Skip if doesn't project onto any cubemap face
                if (!(faceVis[0] || faceVis[1] || faceVis[2] || faceVis[3] || faceVis[4] || faceVis[5])) continue;

                float screenSizePixels = CollisionTester::CalculateSphereScreenSize(
                    worldCollider.center, worldCollider.radius,
                    cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                if (screenSizePixels < 1.0f) continue;

                uint32_t lod = CollisionTester::CalculateLodFromScreenSize(screenSizePixels);
                lod = std::min(lod + POINT_SHADOW_LOD_BIAS, 3u);

                uint32_t allocIndex = data.modelAlloc->meshAllocationOffset + (m * 4) + lod;
                const auto& meshAlloc = drawData->Models.meshAllocations[allocIndex];

                if (meshAlloc.activeTypes[matType])
                {
                    uint32_t indirectIdx = meshAlloc.indirectIndices[pipeIdx][matType];
                    uint32_t isMeshlet = (pipeIdx == static_cast<uint32_t>(PipelineRenderType::Meshlet)) ? 1 : 0;
                    uint32_t drawCallKey = (isMeshlet << 31) | (indirectIdx & 0x7FFFFFFF);

                    uint32_t entityData = static_cast<uint32_t>(data.entity) & 0x7FFFFFFF;
                    if (parentFullyInside) {
                        entityData |= (1u << 31);
                    }

                    for (uint32_t f = 0; f < 6; ++f) {
                        if (faceVis[f]) {
                            PointShadowSortData sortData{
                                .drawCallKey = drawCallKey,
                                .gpuPayload = {
                                    .entityData = entityData,
                                    .lightIndex = (f << 29) | (lightIndex & 0x1FFFFFFF) // [Bit 31-29: Side]
                                }
                            };

                            uint32_t slot = drawData->PointLightShadow.appendedInstanceCount.fetch_add(1, std::memory_order_relaxed);
                            if (slot < _sortBuffer.size()) {
                                _sortBuffer[slot] = sortData;
                            }
                        }
                    }
                }
            }
            };

        // Process Light Loop
        auto processLights = [settings, drawData, lightPool, cullMeshes]
        (const EntityCullData& data, uint32_t lightIndex, IntersectionType chunkVisibility) {
            EntityID lightEntity = drawData->PointLightShadow.visibleLights[lightIndex];
            const auto& lightComp = lightPool->Get(lightEntity);

            IntersectionType visibility = chunkVisibility;

            if (visibility == IntersectionType::Intersect && settings->culling.enableFrustumCulling && settings->culling.enableModelFrustumCulling)
            {
                visibility = CollisionTester::IsInSphereIntersectionType(
                    data.globalWorldCollider,
                    lightComp.position,
                    lightComp.radius
                );

                if (visibility == IntersectionType::Outside)
                    return;
            }
            else if (visibility == IntersectionType::Outside) {
                return;
            }

            bool parentFullyInside = (visibility == IntersectionType::Inside);
            cullMeshes(data, lightIndex, lightComp.position, lightComp.radius, parentFullyInside);
            };

        auto fallbackCull = [withEntityData, processLights, activeShadowLightCount](EntityID entity) {
            withEntityData(entity, [&](const EntityCullData& data) {
                for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx) {
                    processLights(data, lightIdx, IntersectionType::Intersect);
                }
                });
            };

        const auto& staticEntities = transformPool->GetStorage().GetStaticEntities();
        const auto& dynamicEntities = transformPool->GetStorage().GetDynamicEntities();
        const auto& streamEntities = transformPool->GetStorage().GetStreamEntities();

        std::optional<tf::Task> staticTask;
        auto chunkGroup = &drawData->Chunks;
        uint32_t activeChunks = chunkGroup->chunkCounter.load(std::memory_order_relaxed);

        // BVH Chunk Culling Execution
        if (settings->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh && activeChunks > 0)
        {
            if (drawData->PointLightShadow.visibleChunkIds.Size() < activeChunks) {
                drawData->PointLightShadow.visibleChunkIds.Resize(activeChunks);
            }

            drawData->PointLightShadow.visibleChunkCount.store(0, std::memory_order_relaxed);

            staticTask = this->ForEachIndex(uint32_t(0), activeChunks, uint32_t(1), subflow, "Update Static Point Chunks",
                [settings, chunkGroup, staticEntities, drawData, lightPool, activeShadowLightCount, withEntityData, processLights](uint32_t chunkIdx) {
                    const auto& chunk = chunkGroup->chunks[chunkIdx];

                    glm::vec3 extents = (chunk.maxBounds - chunk.minBounds) * 0.5f;
                    GpuMeshCollider chunkCollider;
                    chunkCollider.center = chunk.minBounds + extents;
                    chunkCollider.radius = glm::length(extents);
                    chunkCollider.aabbMin = chunk.minBounds;
                    chunkCollider.aabbMax = chunk.maxBounds;

                    std::vector<IntersectionType> lightVisibilities(activeShadowLightCount, IntersectionType::Intersect);
                    bool isVisibleInAnyLight = false;

                    for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx)
                    {
                        EntityID lightEntity = drawData->PointLightShadow.visibleLights[lightIdx];
                        const auto& lightComp = lightPool->Get(lightEntity);

                        IntersectionType visibility = IntersectionType::Intersect;
                        if (settings->culling.enableFrustumCulling && settings->culling.enableChunkFrustumCulling) {
                            visibility = CollisionTester::IsInSphereIntersectionType(
                                chunkCollider, lightComp.position, lightComp.radius
                            );
                        }

                        lightVisibilities[lightIdx] = visibility;
                        if (visibility != IntersectionType::Outside) {
                            isVisibleInAnyLight = true;
                        }
                    }

                    if (!isVisibleInAnyLight)
                        return;

                    uint32_t slot = drawData->PointLightShadow.visibleChunkCount.fetch_add(1, std::memory_order_relaxed);
                    drawData->PointLightShadow.visibleChunkIds[slot] = chunkIdx;

                    for (uint32_t i = 0; i < chunk.entityCount; ++i) {
                        withEntityData(staticEntities[chunk.firstEntityIndex + i], [&](const EntityCullData& data) {
                            for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx) {
                                if (lightVisibilities[lightIdx] != IntersectionType::Outside) {
                                    processLights(data, lightIdx, lightVisibilities[lightIdx]);
                                }
                            }
                            });
                    }
                });
        }
        else {
            staticTask = this->ForEach(staticEntities, subflow, "Update Static Point Shadow", fallbackCull);
        }

        auto dynamicTask = this->ForEach(dynamicEntities, subflow, "Update Dynamic Point Shadow", fallbackCull);
        auto streamTask = this->ForEach(streamEntities, subflow, "Update Stream Point Shadow", fallbackCull);

        if (staticTask.has_value()) initTask.precede(staticTask.value());
        if (dynamicTask.has_value()) initTask.precede(dynamicTask.value());
        if (streamTask.has_value()) initTask.precede(streamTask.value());

        // Sort Task
        tf::Task sortTask = subflow.emplace([this, drawData]() {
            uint32_t appendedCount = drawData->PointLightShadow.appendedInstanceCount.load(std::memory_order_relaxed);
            if (appendedCount > 0) {
                std::sort(_sortBuffer.begin(), _sortBuffer.begin() + appendedCount);
            }
            }).name("Sort Point Shadow Instances");

        // Finalize Task: Update Indirect Commands and Instance buffer mapping
        tf::Task finalizeTask = subflow.emplace([this, drawData]() {
            uint32_t appendedCount = drawData->PointLightShadow.appendedInstanceCount.load(std::memory_order_relaxed);
            auto& shadowGroup = drawData->PointLightShadow;

            uint32_t currentIndirectIdx = 0xFFFFFFFF;
            uint32_t currentIsMeshlet = 0;

            for (uint32_t i = 0; i < appendedCount; ++i)
            {
                const auto& item = _sortBuffer[i];
                uint32_t isMeshlet = (item.drawCallKey >> 31) & 1;
                uint32_t indirectIdx = item.drawCallKey & 0x7FFFFFFF;

                shadowGroup.instances[i] = item.gpuPayload;

                if (indirectIdx != currentIndirectIdx) {
                    currentIndirectIdx = indirectIdx;
                    currentIsMeshlet = isMeshlet;
                    shadowGroup.shadowDescriptors[indirectIdx].instanceOffset = i;
                }

                if (isMeshlet) {
                    shadowGroup.meshletCmds[indirectIdx].groupCountX += 1;
                }
                else {
                    shadowGroup.traditionalCmds[indirectIdx].instanceCount += 1;
                }
            }
            }).name("Finalize Point Shadow Indirects");

        if (staticTask.has_value()) staticTask.value().precede(sortTask);
        if (dynamicTask.has_value()) dynamicTask.value().precede(sortTask);
        if (streamTask.has_value()) streamTask.value().precede(sortTask);

        sortTask.precede(finalizeTask);
    }

    void PointLightShadowCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            bool needsCommandUpload = (settings->culling.pointLightCullingDevice == CullingDeviceType::CPU && settings->culling.pointLightShadowCullingDevice == CullingDeviceType::CPU) || (drawData->syncFramesRemaining.load(std::memory_order_relaxed) > 0);

            auto& mainGroup = drawData->Models;
            auto& shadowGroup = drawData->PointLightShadow;

            if (settings->culling.pointLightCullingDevice == CullingDeviceType::CPU && settings->culling.pointLightShadowCullingDevice == CullingDeviceType::CPU)
            {
                uint32_t appendedCount = shadowGroup.appendedInstanceCount.load(std::memory_order_relaxed);

                if (appendedCount > 0) {
                    shadowGroup.instanceBuffer.Write(frameIndex, shadowGroup.instances.Data(), appendedCount * sizeof(PointShadowInstancePayload), 0);
                }
            }

            if (needsCommandUpload)
            {
                uint32_t commandCount = shadowGroup.totalCommandCount;
                if (commandCount > 0) {
                    shadowGroup.descriptorBuffer.Write(frameIndex, shadowGroup.shadowDescriptors.Data(), commandCount * sizeof(MeshDrawDescriptor), 0);
                }

                size_t tradSize = mainGroup.activeTraditionalCount * sizeof(VkDrawIndirectCommand);
                if (tradSize > 0) {
                    shadowGroup.indirectBuffer.Write(frameIndex, shadowGroup.traditionalCmds.Data(), tradSize, 0);
                }

                size_t meshletSize = mainGroup.activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
                if (meshletSize > 0) {
                    size_t meshletGpuOffset = tradSize;
                    shadowGroup.indirectBuffer.Write(frameIndex, shadowGroup.meshletCmds.Data(), meshletSize, meshletGpuOffset);
                }
            }
            });
    }
}