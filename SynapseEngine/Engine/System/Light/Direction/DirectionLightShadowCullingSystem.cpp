#include "DirectionLightShadowCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "DirectionLightShadowRenderSystem.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "Engine/System/Rendering/AnimationSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"
#include "Engine/System/Core/StaticSpatialSahSystem.h"

#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include <atomic>
#include "DirectionLightCullingSystem.h"
#include "DirectionLightShadowAtlasSystem.h"
#include "DirectionLightShadowSystem.h"

#include "Engine/Component/Core/TagComponent.h"
#include "Engine/System/Core/TagSystem.h"

namespace Syn
{
	constexpr bool ENABLE_DEBUG_LOGGING = false;

    struct LightVis {
        bool isVisible = false;
        std::array<IntersectionType, CASCADES_PER_LIGHT> cascadeVis;
    };

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

    std::vector<TypeID> DirectionLightShadowCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<DirectionLightShadowRenderSystem>::ID,
            TypeInfo<DirectionLightCullingSystem>::ID,
            TypeInfo<DirectionLightShadowAtlasSystem>::ID,
            TypeInfo<DirectionLightShadowSystem>::ID,
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

    void DirectionLightShadowCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();

        // Reset CPU counters
        tf::Task initTask = this->EmplaceTask(subflow, "Update Init", [drawData]() {
            auto& shadowGroup = drawData->DirectionLightShadow;
            auto& mainGroup = drawData->Models;

            for (uint32_t i = 0; i < mainGroup.activeTraditionalCount; ++i) {
                shadowGroup.traditionalCmds[i].instanceCount = 0;
                shadowGroup.paddedTraditionalCounts[i * 16] = 0;
            }
            for (uint32_t i = 0; i < mainGroup.activeMeshletCount; ++i) {
                shadowGroup.meshletCmds[i].groupCountX = 0;
                shadowGroup.paddedMeshletCounts[i * 16] = 0;
            }
            });

        if (settings->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU) {
            return;
        }

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto animPool = registry->GetPool<AnimationComponent>();
        auto overridePool = registry->GetPool<MaterialOverrideComponent>();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();
        auto tagPool = registry->GetPool<TagComponent>();

        EntityID cameraEntity = scene->GetSceneCameraEntity();
        if (!modelPool || !transformPool || !cameraPool || cameraEntity == NULL_ENTITY || !shadowPool)
            return;

        uint32_t activeShadowLightCount = drawData->DirectionLightShadow.visibleLightCount;
        if (activeShadowLightCount == 0)
            return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto modelManager = ServiceLocator::GetModelManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto modelSnapshot = modelManager->GetResourceSnapshot();
        auto animSnapshot = animationManager->GetResourceSnapshot();
        auto matTypeSnapshot = materialManager->GetRenderTypeSnapshot();

        // Extract entity properties (runs exactly once per entity)
        auto withEntityData = [modelPool, transformPool, animPool, overridePool, modelSnapshot, tagPool, animSnapshot, drawData]
            (EntityID entity, auto&& nextFunc) {
                if (tagPool && tagPool->Has(entity)) {
                    const auto& tag = tagPool->Get(entity);
                    if (!tag.globalEnabled || !tag.castShadow) {
                        return;
                    }
                }

                if (!modelPool->Has(entity))
                    return;

                const auto& modelComp = modelPool->Get(entity);
                if (modelComp.modelIndex == NULL_INDEX || modelComp.modelIndex >= drawData->Models.modelAllocations.Size())
                    return;

                const auto& snapshotEntry = modelSnapshot[modelComp.modelIndex];
                if (snapshotEntry.resource == nullptr || snapshotEntry.state != ResourceState::Ready)
                    return;

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

                // Calculate Global World Collider
                GpuMeshCollider globalLocalCollider = modelResource->cpuData.globalCollider;

                if (hasAnimation) {
                    globalLocalCollider = animResource->cpuData.frameGlobalColliders[animFrameIndex];
                }

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
                    .materialOverrides = overrides
                };

                nextFunc(data);
            };


        //Mesh-level culling and indirect command dispatch
        auto cullMeshes = [settings, drawData, matTypeSnapshot, cameraComp, screenRes]
        (const EntityCullData& data, uint32_t lightIndex, uint32_t cascadeIdx, const FrustumCollider& frustum, bool parentFullyInside) {
            for (uint32_t m = 0; m < data.meshCount; ++m)
            {
                uint32_t matIdx = data.modelResource->cpuData.meshMaterialIndices[m];
                if (!data.materialOverrides.empty() && m < data.materialOverrides.size() && data.materialOverrides[m] != UINT32_MAX)
                    matIdx = data.materialOverrides[m];

                MaterialRenderType matType = (matIdx < matTypeSnapshot.size()) ? matTypeSnapshot[matIdx] : MaterialRenderType::Opaque1Sided;

                // Process opaque materials only
                if (matType != MaterialRenderType::Opaque1Sided && matType != MaterialRenderType::Opaque2Sided) {
                    continue;
                }

                bool isVisible = true;
                GpuMeshCollider worldCollider;

                if (data.meshCount > 1)
                {
                    GpuMeshCollider localCollider;

                    if (data.hasAnimation) {
                        uint32_t frameOffset = data.animFrameIndex * data.animResource->cpuData.descriptor.globalMeshCount;
                        localCollider = data.animResource->cpuData.frameMeshColliders[frameOffset + m];
                    }
                    else {
                        localCollider = data.modelResource->cpuData.meshColliders[m];
                    }

                    worldCollider = MeshUtils::TransformCollider(localCollider, data.transform);

                    if (!parentFullyInside && settings->culling.enableFrustumCulling && settings->culling.enableMeshFrustumCulling)
                        isVisible = CollisionTester::IsInFrustum(worldCollider, frustum);
                }
                else {
                    worldCollider = data.globalWorldCollider;
                }

                if (isVisible)
                {
                    // LOD computed from main camera view to save vertex processing in shadows
                    float screenSizePixels = CollisionTester::CalculateSphereScreenSize(
                        worldCollider.center, worldCollider.radius,
                        cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                    if (screenSizePixels < 1.0f)
                        continue;

                    uint32_t lod = CollisionTester::CalculateLodFromScreenSize(screenSizePixels);
                    lod = std::min(lod + SHADOW_LOD_BIAS, 3u);

                    uint32_t allocIndex = data.modelAlloc->meshAllocationOffset + (m * 4) + lod;
                    const auto& meshAlloc = drawData->Models.meshAllocations[allocIndex];

                    if (meshAlloc.activeTypes[matType])
                    {
                        uint32_t slotIndex = 0;
                        uint32_t indirectIdx = meshAlloc.indirectIndices[matType];

                        // Increment atomic padded counters
                        if (meshAlloc.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET) {
                            std::atomic_ref<uint32_t> countRef(drawData->DirectionLightShadow.paddedMeshletCounts[indirectIdx * 16]);
                            slotIndex = countRef.fetch_add(1, std::memory_order_relaxed);
                        }
                        else {
                            std::atomic_ref<uint32_t> countRef(drawData->DirectionLightShadow.paddedTraditionalCounts[indirectIdx * 16]);
                            slotIndex = countRef.fetch_add(1, std::memory_order_relaxed);
                        }

                        // Write Bit-Packed Payload
                        uint32_t bufferIndex = (meshAlloc.instanceOffsets[matType] * SHADOW_MULTIPLIER) + slotIndex;
                        if (bufferIndex < drawData->DirectionLightShadow.instances.Size())
                        {
                            // [Bit 31: FullyInside (1 bit)] [Bits 28-30: LightIdx (3 bit)] [Bits 26-27: CascadeIdx (2 bit)] [Bits 0-25: EntityID (26 bit)]

                            uint32_t payload = static_cast<uint32_t>(data.entity) & 0x3FFFFFF;
                            payload |= (cascadeIdx & 0x3) << 26;
                            payload |= (lightIndex & 0x7) << 28;

                            if (parentFullyInside) {
                                payload |= (1u << 31);
                            }
                            else {
                                payload &= ~(1u << 31);
                            }

                            drawData->DirectionLightShadow.instances[bufferIndex] = payload;

                            if constexpr (ENABLE_DEBUG_LOGGING) {
                                Info("Shadow Cull - Entity: {}, LightIdx: {}, CascadeIdx: {}, LOD: {}, ScreenSize: {:.2f}, BufferIndex: {}",
                                    static_cast<uint32_t>(data.entity), lightIndex, cascadeIdx, lod, screenSizePixels, bufferIndex);
                            }
                        }
                    }
                }
            }
            };


        //Process the 4 cascades for a specific light
        auto cullLightCascades = [settings, drawData, shadowPool, cullMeshes]
        (const EntityCullData& data, uint32_t lightIndex, const std::span<IntersectionType> chunkVisibilities) {
            EntityID lightEntity = drawData->DirectionLightShadow.visibleLights[lightIndex];
            const auto& shadowComp = shadowPool->Get(lightEntity);

            for (uint32_t cascadeIdx = 0; cascadeIdx < 4; ++cascadeIdx)
            {
                IntersectionType visibility = chunkVisibilities[cascadeIdx];

                if (visibility == IntersectionType::Intersect && settings->culling.enableFrustumCulling && settings->culling.enableModelFrustumCulling) {
                    visibility = CollisionTester::IsInFrustumIntersectionType(data.globalWorldCollider, shadowComp.cascadeFrustums[cascadeIdx]);

                    if (visibility == IntersectionType::Outside)
                        continue;
                }
                else if (visibility == IntersectionType::Outside) {
                    continue;
                }

                bool parentFullyInside = (visibility == IntersectionType::Inside);

                cullMeshes(data, lightIndex, cascadeIdx, shadowComp.cascadeFrustums[cascadeIdx], parentFullyInside);
            }
            };


        //Fallback for streaming, dynamic, or unchunked entities
        auto fallbackCull = [withEntityData, cullLightCascades, activeShadowLightCount]
        (EntityID entity) {
            withEntityData(entity, [&](const EntityCullData& data) {

                std::array<IntersectionType, 4> defaultVis;
                defaultVis.fill(IntersectionType::Intersect);

                for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx) {
                    cullLightCascades(data, lightIdx, defaultVis);
                }
                });
            };


        const auto& staticEntities = transformPool->GetStorage().GetStaticEntities();
        const auto& dynamicEntities = transformPool->GetStorage().GetDynamicEntities();
        const auto& streamEntities = transformPool->GetStorage().GetStreamEntities();

        std::optional<tf::Task> staticTask;
        auto chunkGroup = &drawData->Chunks;

        uint32_t activeChunks = chunkGroup->chunkCounter.load(std::memory_order_relaxed);

        // Static Bvh Chunk Culling Execution
        if (settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh && activeChunks > 0)
        {
            if (drawData->DirectionLightShadow.visibleChunkIds.Size() < activeChunks) {
                drawData->DirectionLightShadow.visibleChunkIds.Resize(activeChunks);
            }

            drawData->DirectionLightShadow.visibleChunkCount.store(0, std::memory_order_relaxed);

            staticTask = this->ForEachIndex(uint32_t(0), activeChunks, uint32_t(1), subflow, "Update Static Shadow Chunks",
                [settings, chunkGroup, staticEntities, drawData, shadowPool, activeShadowLightCount, withEntityData, cullLightCascades](uint32_t chunkIdx) {
                    const auto& chunk = chunkGroup->chunks[chunkIdx];

                    glm::vec3 extents = (chunk.maxBounds - chunk.minBounds) * 0.5f;
                    GpuMeshCollider chunkCollider;
                    chunkCollider.center = chunk.minBounds + extents;
                    chunkCollider.radius = glm::length(extents);
                    chunkCollider.aabbMin = chunk.minBounds;
                    chunkCollider.aabbMax = chunk.maxBounds;

                    // Allocate light visibilities purely for the hot path of this specific chunk
                    std::array<LightVis, MAX_DIR_LIGHTS> lightVisibilities;
                    bool isVisibleInAnyLight = false;

                    for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx)
                    {
                        EntityID lightEntity = drawData->DirectionLightShadow.visibleLights[lightIdx];
                        const auto& shadowComp = shadowPool->Get(lightEntity);

                        for (uint32_t cascadeIdx = 0; cascadeIdx < 4; ++cascadeIdx)
                        {
                            IntersectionType visibility = IntersectionType::Intersect;

                            if (settings->culling.enableFrustumCulling && settings->culling.enableChunkFrustumCulling)
                            {
                                visibility = CollisionTester::IsInFrustumIntersectionType(
                                    chunkCollider, shadowComp.cascadeFrustums[cascadeIdx]
                                );
                            }

                            lightVisibilities[lightIdx].cascadeVis[cascadeIdx] = visibility;

                            if (visibility != IntersectionType::Outside) {
                                lightVisibilities[lightIdx].isVisible = true;
                                isVisibleInAnyLight = true;
                            }
                        }
                    }

                    // Prune chunk entirely if no directional light cascade intersects it
                    if (!isVisibleInAnyLight) 
                        return;

                    // Register chunk as visible for shadow pass
                    uint32_t slot = drawData->DirectionLightShadow.visibleChunkCount.fetch_add(1, std::memory_order_relaxed);
                    drawData->DirectionLightShadow.visibleChunkIds[slot] = chunkIdx;

                    for (uint32_t i = 0; i < chunk.entityCount; ++i) {
                        EntityID entity = staticEntities[chunk.firstEntityIndex + i];

                        withEntityData(entity, [&](const EntityCullData& data) {
                            for (uint32_t lightIdx = 0; lightIdx < activeShadowLightCount; ++lightIdx) {
                                if (lightVisibilities[lightIdx].isVisible) {
                                    cullLightCascades(data, lightIdx, lightVisibilities[lightIdx].cascadeVis);
                                }
                            }
                            });
                    }
                });
        }
        else
        {
            staticTask = this->ForEach(staticEntities, subflow, "Update Static Shadow", fallbackCull);
        }

        auto dynamicTask = this->ForEach(dynamicEntities, subflow, "Update Dynamic Shadow", fallbackCull);
        auto streamTask = this->ForEach(streamEntities, subflow, "Update Stream Shadow", fallbackCull);

        if (staticTask.has_value()) initTask.precede(staticTask.value());
        if (dynamicTask.has_value()) initTask.precede(dynamicTask.value());
        if (streamTask.has_value()) initTask.precede(streamTask.value());
    }

    void DirectionLightShadowCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            bool needsCommandUpload = (settings->culling.directionLightShadowCullingDevice == CPU) || (drawData->syncFramesRemaining.load(std::memory_order_relaxed) > 0);

            auto& mainGroup = drawData->Models;
            auto& shadowGroup = drawData->DirectionLightShadow;

            if (settings->culling.directionLightShadowCullingDevice == CPU)
            {
                // Sync CPU counters to indirect commands
                for (uint32_t i = 0; i < mainGroup.activeTraditionalCount; ++i) {
                    shadowGroup.traditionalCmds[i].instanceCount = shadowGroup.paddedTraditionalCounts[i * 16];
                }

                for (uint32_t i = 0; i < mainGroup.activeMeshletCount; ++i) {
                    shadowGroup.meshletCmds[i].groupCountX = shadowGroup.paddedMeshletCounts[i * 16];
                }

                // Upload instances
                size_t instanceSize = mainGroup.totalAllocatedInstances * SHADOW_MULTIPLIER * sizeof(uint32_t);
                if (instanceSize > 0) {
                    shadowGroup.instanceBuffer.Write(frameIndex, shadowGroup.instances.Data(), instanceSize, 0);
                }
            }

            if (needsCommandUpload)
            {
                // Upload base draw commands (indirect data)
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