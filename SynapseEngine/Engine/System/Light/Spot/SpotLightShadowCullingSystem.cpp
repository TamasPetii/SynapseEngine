#include "SpotLightShadowCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Logger/SynLog.h"
#include "SpotLightShadowRenderSystem.h"
#include "SpotLightCullingSystem.h"
#include "SpotLightShadowAtlasSystem.h"
#include "SpotLightSystem.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "Engine/System/Rendering/AnimationSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"
#include "Engine/System/Core/StaticSpatialSahSystem.h"

#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
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
    };

    struct ConeParams {
        glm::vec3 pos;
        glm::vec3 dir;
        float range;
        float cosAngle;
        float sinAngle;
    };

    std::vector<TypeID> SpotLightShadowCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<SpotLightShadowRenderSystem>::ID,
            TypeInfo<SpotLightCullingSystem>::ID,
            TypeInfo<SpotLightShadowAtlasSystem>::ID,
            TypeInfo<SpotLightSystem>::ID,
            TypeInfo<TransformSystem>::ID,
            TypeInfo<ModelSystem>::ID,
            TypeInfo<RenderSystem>::ID,
            TypeInfo<AnimationSystem>::ID,
            TypeInfo<MaterialSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<StaticSpatialSahSystem>::ID,
            TypeInfo<TagSystem>::ID
        };
    }

    void SpotLightShadowCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();

        tf::Task initTask = this->EmplaceTask(subflow, "Spot Shadow Update Init", [this, drawData]() {
            auto& shadowGroup = drawData->SpotLightShadow;
            auto& mainGroup = drawData->Models;

            shadowGroup.appendedInstanceCount.store(0, std::memory_order_relaxed);

            for (uint32_t i = 0; i < mainGroup.activeTraditionalCount; ++i) {
                shadowGroup.traditionalCmds[i].instanceCount = 0;
            }
            for (uint32_t i = 0; i < mainGroup.activeMeshletCount; ++i) {
                shadowGroup.meshletCmds[i].groupCountX = 0;
            }

            uint32_t expectedMaxInstances = mainGroup.totalAllocatedInstances * SPOT_SHADOW_MULTIPLIER;
            if (_sortBuffer.size() < expectedMaxInstances) {
                _sortBuffer.resize(expectedMaxInstances);
            }
            });

        if (settings->culling.spotLightCullingDevice == CullingDeviceType::GPU)
            return;

        if (settings->culling.spotLightShadowCullingDevice == CullingDeviceType::GPU)
            return;

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto animPool = registry->GetPool<AnimationComponent>();
        auto overridePool = registry->GetPool<MaterialOverrideComponent>();
        auto shadowPool = registry->GetPool<SpotLightShadowComponent>();
        auto spotLightPool = registry->GetPool<SpotLightComponent>();
        auto tagPool = registry->GetPool<TagComponent>();

        EntityID cameraEntity = scene->GetSceneCameraEntity();
        if (!modelPool || !transformPool || !cameraPool || cameraEntity == NULL_ENTITY || !shadowPool || !spotLightPool) return;

        uint32_t activeShadowLightCount = drawData->SpotLightShadow.visibleLightCount;
        if (activeShadowLightCount == 0) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto modelManager = ServiceLocator::GetModelManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto modelSnapshot = modelManager->GetResourceSnapshot();
        auto animSnapshot = animationManager->GetResourceSnapshot();
        auto matTypeSnapshot = materialManager->GetRenderTypeSnapshot();

        // Extract Entity Data (Runs once per entity)
        auto withEntityData = [modelPool, transformPool, animPool, overridePool, modelSnapshot, tagPool, animSnapshot, drawData]
        (EntityID entity, auto&& nextFunc) {
            if (tagPool && tagPool->Has(entity)) {
                const auto& tag = tagPool->Get(entity);
                if (!tag.globalEnabled || !tag.castShadow) {
                    return;
                }
            }

            if (!modelPool->Has(entity)) return;

            const auto& modelComp = modelPool->Get(entity);
            if (modelComp.modelIndex == NULL_INDEX || modelComp.modelIndex >= drawData->Models.modelAllocations.Size()) return;

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

            const StaticMesh* modelResource = static_cast<const StaticMesh*>(snapshotEntry.resource.get());

            GpuMeshCollider globalLocalCollider = hasAnimation ?
                animResource->cpuData.frameGlobalColliders[animFrameIndex] : modelResource->cpuData.globalCollider;

            GpuMeshCollider worldCollider = MeshUtils::TransformCollider(globalLocalCollider, transformComp.transform);

            EntityCullData data{
                .entity = entity, .transform = transformComp.transform, .globalWorldCollider = worldCollider,
                .meshCount = modelAlloc.meshAllocationCount / 4, .modelResource = modelResource,
                .modelAlloc = &modelAlloc, .hasAnimation = hasAnimation, .animFrameIndex = animFrameIndex,
                .animResource = animResource, .materialOverrides = overrides
            };
            nextFunc(data);
            };

        // Mesh Level Cone Culling & Sort Payload Generation
        auto cullMeshes = [this, settings, drawData, matTypeSnapshot, cameraComp, screenRes]
        (const EntityCullData& data, uint32_t lightIndex, const ConeParams& cone, bool parentFullyInside) {

            for (uint32_t m = 0; m < data.meshCount; ++m)
            {
                uint32_t matIdx = data.modelResource->cpuData.meshMaterialIndices[m];
                if (!data.materialOverrides.empty() && m < data.materialOverrides.size() && data.materialOverrides[m] != UINT32_MAX)
                    matIdx = data.materialOverrides[m];

                MaterialRenderType matType = (matIdx < matTypeSnapshot.size()) ? matTypeSnapshot[matIdx] : MaterialRenderType::Opaque1Sided;

                if (matType != MaterialRenderType::Opaque1Sided && matType != MaterialRenderType::Opaque2Sided) continue;

                bool isVisible = true;
                GpuMeshCollider worldCollider;

                if (data.meshCount > 1) {
                    GpuMeshCollider localCollider = data.hasAnimation ?
                        data.animResource->cpuData.frameMeshColliders[(data.animFrameIndex * data.animResource->cpuData.descriptor.globalMeshCount) + m] :
                        data.modelResource->cpuData.meshColliders[m];

                    worldCollider = MeshUtils::TransformCollider(localCollider, data.transform);

                    if (!parentFullyInside && settings->culling.enableFrustumCulling && settings->culling.enableMeshFrustumCulling) {
                        isVisible = CollisionTester::TestConeSphere(
                            cone.pos, cone.dir, cone.range, cone.cosAngle, cone.sinAngle,
                            worldCollider.center, worldCollider.radius
                        );
                    }
                }
                else {
                    worldCollider = data.globalWorldCollider;
                }

                if (isVisible)
                {
                    float screenSizePixels = CollisionTester::CalculateSphereScreenSize(
                        worldCollider.center, worldCollider.radius,
                        cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                    if (screenSizePixels < 1.0f) continue;

                    uint32_t lod = CollisionTester::CalculateLodFromScreenSize(screenSizePixels);
                    lod = std::min(lod + SPOT_SHADOW_LOD_BIAS, 3u);

                    uint32_t allocIndex = data.modelAlloc->meshAllocationOffset + (m * 4) + lod;
                    const auto& meshAlloc = drawData->Models.meshAllocations[allocIndex];

                    if (meshAlloc.activeTypes[matType])
                    {
                        uint32_t indirectIdx = meshAlloc.indirectIndices[matType];
                        uint32_t isMeshlet = (meshAlloc.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET) ? 1 : 0;
                        uint32_t drawCallKey = (isMeshlet << 31) | (indirectIdx & 0x7FFFFFFF);

                        uint32_t entityData = static_cast<uint32_t>(data.entity) & 0x7FFFFFFF;
                        if (parentFullyInside) {
                            entityData |= (1u << 31);
                        }

                        SpotShadowSortData sortData{
                            .drawCallKey = drawCallKey,
                            .gpuPayload = {
                                .entityData = entityData,
                                .lightIndex = lightIndex
                            }
                        };

                        uint32_t slot = drawData->SpotLightShadow.appendedInstanceCount.fetch_add(1, std::memory_order_relaxed);
                        if (slot < _sortBuffer.size()) {
                            _sortBuffer[slot] = sortData;
                        }
                    }
                }
            }
            };

        // Process Light Loop (Per Entity vs Light Cone)
        auto processLights = [settings, drawData, spotLightPool, cullMeshes]
        (const EntityCullData& data, uint32_t lightIndex, IntersectionType chunkVisibility) {
            EntityID lightEntity = drawData->SpotLightShadow.visibleLights[lightIndex];
            const auto& lightComp = spotLightPool->Get(lightEntity);

            float outerRad = glm::radians(lightComp.outerAngle);
            ConeParams cone{
                .pos = lightComp.position,
                .dir = lightComp.direction,
                .range = lightComp.range,
                .cosAngle = std::cos(outerRad),
                .sinAngle = std::sin(outerRad)
            };

            IntersectionType visibility = chunkVisibility;

            if (visibility == IntersectionType::Intersect && settings->culling.enableFrustumCulling && settings->culling.enableModelFrustumCulling) 
            {
                visibility = CollisionTester::TestConeSphereIntersectionType(
                    cone.pos, cone.dir, cone.range, cone.cosAngle, cone.sinAngle,
                    data.globalWorldCollider.center, data.globalWorldCollider.radius
                );

                if (visibility == IntersectionType::Outside) 
                    return;
            }
            else if (visibility == IntersectionType::Outside) {
                return;
            }

            bool parentFullyInside = (visibility == IntersectionType::Inside);
            cullMeshes(data, lightIndex, cone, parentFullyInside);
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
        if (settings->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh && activeChunks > 0)
        {
            if (drawData->SpotLightShadow.visibleChunkIds.Size() < activeChunks) {
                drawData->SpotLightShadow.visibleChunkIds.Resize(activeChunks);
            }

            drawData->SpotLightShadow.visibleChunkCount.store(0, std::memory_order_relaxed);

            staticTask = this->ForEachIndex(uint32_t(0), activeChunks, uint32_t(1), subflow, "Update Static Spot Chunks",
                [settings, chunkGroup, staticEntities, drawData, spotLightPool, activeShadowLightCount, withEntityData, processLights](uint32_t chunkIdx) {
                    const auto& chunk = chunkGroup->chunks[chunkIdx];

                    // Calculate conservative bounding sphere for the AABB chunk
                    glm::vec3 extents = (chunk.maxBounds - chunk.minBounds) * 0.5f;
                    glm::vec3 chunkCenter = chunk.minBounds + extents;
                    float chunkRadius = glm::length(extents);

                    std::vector<IntersectionType> lightVisibilities(activeShadowLightCount, IntersectionType::Intersect);
                    bool isVisibleInAnyLight = false;

                    for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx)
                    {
                        EntityID lightEntity = drawData->SpotLightShadow.visibleLights[lightIdx];
                        const auto& lightComp = spotLightPool->Get(lightEntity);

                        IntersectionType visibility = IntersectionType::Intersect;
                        if (settings->culling.enableFrustumCulling && settings->culling.enableChunkFrustumCulling) {
                            float outerRad = glm::radians(lightComp.outerAngle);
                            visibility = CollisionTester::TestConeSphereIntersectionType(
                                lightComp.position, lightComp.direction, lightComp.range,
                                std::cos(outerRad), std::sin(outerRad),
                                chunkCenter, chunkRadius
                            );
                        }

                        lightVisibilities[lightIdx] = visibility;
                        if (visibility != IntersectionType::Outside) {
                            isVisibleInAnyLight = true;
                        }
                    }

                    if (!isVisibleInAnyLight) 
                        return;

                    uint32_t slot = drawData->SpotLightShadow.visibleChunkCount.fetch_add(1, std::memory_order_relaxed);
                    drawData->SpotLightShadow.visibleChunkIds[slot] = chunkIdx;

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
            staticTask = this->ForEach(staticEntities, subflow, "Update Static Spot Shadow", fallbackCull);
        }

        auto dynamicTask = this->ForEach(dynamicEntities, subflow, "Update Dynamic Spot Shadow", fallbackCull);
        auto streamTask = this->ForEach(streamEntities, subflow, "Update Stream Spot Shadow", fallbackCull);

        if (staticTask.has_value()) initTask.precede(staticTask.value());
        if (dynamicTask.has_value()) initTask.precede(dynamicTask.value());
        if (streamTask.has_value()) initTask.precede(streamTask.value());

        // Sort Task
        tf::Task sortTask = subflow.emplace([this, drawData]() {
            uint32_t appendedCount = drawData->SpotLightShadow.appendedInstanceCount.load(std::memory_order_relaxed);
            if (appendedCount > 0) {
                std::sort(_sortBuffer.begin(), _sortBuffer.begin() + appendedCount);
            }
            }).name("Sort Spot Shadow Instances");

        // Finalize Task: Update Indirect Commands and Instance buffer mapping
        tf::Task finalizeTask = subflow.emplace([this, drawData]() {
            uint32_t appendedCount = drawData->SpotLightShadow.appendedInstanceCount.load(std::memory_order_relaxed);
            auto& shadowGroup = drawData->SpotLightShadow;

            uint32_t currentIndirectIdx = 0xFFFFFFFF;
            uint32_t currentIsMeshlet = 0;

            auto logPreviousBatch = [&]() {
                if constexpr (ENABLE_DEBUG_LOGGING) {
                    if (currentIndirectIdx != 0xFFFFFFFF) {
                        uint32_t count = currentIsMeshlet ?
                            shadowGroup.meshletCmds[currentIndirectIdx].groupCountX :
                            shadowGroup.traditionalCmds[currentIndirectIdx].instanceCount;

                        Info("SpotShadow Finalize - IndirectIdx: {} | Offset: {} | Count: {} | Type: {}",
                            currentIndirectIdx,
                            shadowGroup.shadowDescriptors[currentIndirectIdx].instanceOffset,
                            count,
                            currentIsMeshlet ? "Meshlet" : "Traditional");
                    }
                }
                };

            for (uint32_t i = 0; i < appendedCount; ++i)
            {
                const auto& item = _sortBuffer[i];
                uint32_t isMeshlet = (item.drawCallKey >> 31) & 1;
                uint32_t indirectIdx = item.drawCallKey & 0x7FFFFFFF;

                shadowGroup.instances[i] = item.gpuPayload;

                if (indirectIdx != currentIndirectIdx) {
                    logPreviousBatch();

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
            }).name("Finalize Spot Shadow Indirects");

        if (staticTask.has_value()) staticTask.value().precede(sortTask);
        if (dynamicTask.has_value()) dynamicTask.value().precede(sortTask);
        if (streamTask.has_value()) streamTask.value().precede(sortTask);

        sortTask.precede(finalizeTask);
    }

    void SpotLightShadowCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            bool needsCommandUpload = (settings->culling.spotLightCullingDevice == CullingDeviceType::CPU && settings->culling.spotLightShadowCullingDevice == CullingDeviceType::CPU) || (drawData->syncFramesRemaining.load(std::memory_order_relaxed) > 0);

            auto& mainGroup = drawData->Models;
            auto& shadowGroup = drawData->SpotLightShadow;

            if (settings->culling.spotLightCullingDevice == CullingDeviceType::CPU && settings->culling.spotLightShadowCullingDevice == CullingDeviceType::CPU)
            {
                uint32_t appendedCount = shadowGroup.appendedInstanceCount.load(std::memory_order_relaxed);

                if (appendedCount > 0) {
                    shadowGroup.instanceBuffer.Write(frameIndex, shadowGroup.instances.Data(), appendedCount * sizeof(SpotShadowInstancePayload), 0);
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