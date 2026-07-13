#include "ModelFrustumCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/System/Rendering/AnimationSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"
#include "Engine/System/Core/StaticSpatialSahSystem.h"
#include "Engine/System/Core/TagSystem.h"

#include "Engine/Component/Rendering/MaterialOverrideComponent.h"

#include "Engine/Mesh/Utils/MeshUtils.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include <glm/gtc/matrix_access.hpp>
#include <span>
#include <atomic>

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    std::vector<TypeID> ModelFrustumCullingSystem::GetReadDependencies() const {
        return { 
            TypeInfo<ModelSystem>::ID,
            TypeInfo<TransformSystem>::ID,
            TypeInfo<RenderSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<AnimationSystem>::ID,
            TypeInfo<MaterialSystem>::ID,
            TypeInfo<StaticSpatialSahSystem>::ID,
            TypeInfo<TagSystem>::ID
        };
    }

    void ModelFrustumCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto settings = scene->GetSettings();
        auto drawData = scene->GetSceneDrawData();
        auto matTypeSnapshot = scene->GetSystemContext().materialRenderTypes;
        auto overridePool = scene->GetRegistry()->GetPool<MaterialOverrideComponent>();

        tf::Task initTask = this->EmplaceTask(subflow, "Update Init", [drawData]() {
            for (uint32_t i = 0; i < drawData->Models.activeTraditionalCount; ++i) {
                drawData->Models.traditionalCmds[i].instanceCount = 0;
                drawData->Models.paddedTraditionalCounts[i * 16] = 0;
            }
            for (uint32_t i = 0; i < drawData->Models.activeMeshletCount; ++i) {
                drawData->Models.meshletCmds[i].groupCountX = 0;
                drawData->Models.paddedMeshletCounts[i * 16] = 0;
            }

            drawData->Debug.modelAabbCmdTemplate.instanceCount = 0;
            drawData->Debug.modelSphereCmdTemplate.instanceCount = 0;
            });

        if (settings->culling.geometryCullingDevice == CullingDeviceType::GPU) {
            return;
        }

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto animPool = registry->GetPool<AnimationComponent>();
        auto tagPool = registry->GetPool<TagComponent>();

        EntityID cameraEntity = scene->GetSceneCameraEntity();
        if (!modelPool || !transformPool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        auto& modelSnapshot = scene->GetSystemContext().modelSnapshots;
        auto& animSnapshot = scene->GetSystemContext().animationSnapshots;

        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto cullFunc = [settings, drawData, modelPool, transformPool, modelSnapshot, cameraComp, animPool, tagPool, animSnapshot, matTypeSnapshot, overridePool, screenRes]
        (EntityID entity, IntersectionType chunkVisibility) {
            if (tagPool && tagPool->Has(entity)) {
                if (!tagPool->Get(entity).globalEnabled) {
                    return;
                }
            }
            
            const FrustumCollider& frustum = cameraComp.frustum;
            const auto& transformComp = transformPool->Get(entity);

            if (!modelPool->Has(entity))
                return;

            const auto& modelComp = modelPool->Get(entity);

            if (modelComp.modelIndex == NULL_INDEX)
                return;

            if (modelComp.modelIndex >= drawData->Models.modelAllocations.Size())
                return;

            const auto& snapshotEntry = modelSnapshot[modelComp.modelIndex];
            if (snapshotEntry.resource == nullptr || snapshotEntry.state != ResourceState::Ready)
                return;

            const auto& modelAlloc = drawData->Models.modelAllocations[modelComp.modelIndex];

            uint32_t meshCount = modelAlloc.meshAllocationCount / 4;
            auto resource = snapshotEntry.resource;
            const glm::mat4& transform = transformComp.transform;

            bool hasAnimation = false;
            uint32_t animFrameIndex = 0;
            std::shared_ptr<Animation> animResource = nullptr;

            if (animPool && animPool->Has(entity)) {
                const auto& animComp = animPool->Get(entity);
                if (animComp.isReady && animComp.animationIndex != NULL_INDEX && animComp.animationIndex < animSnapshot.size()) {
                    const auto& aSnapshotEntry = animSnapshot[animComp.animationIndex];
                    if (aSnapshotEntry.resource != nullptr && aSnapshotEntry.state == ResourceState::Ready) {
                        hasAnimation = true;
                        animFrameIndex = animComp.frameIndex;
                        animResource = aSnapshotEntry.resource;
                    }
                }
            }

            GpuMeshCollider globalLocalCollider = resource->cpuData.globalCollider;

            if (hasAnimation) {
                globalLocalCollider = animResource->cpuData.frameGlobalColliders[animFrameIndex];
            }

            GpuMeshCollider globalWorldCollider = MeshUtils::TransformCollider(globalLocalCollider, transform);

            IntersectionType visibility = chunkVisibility;
            if (visibility == IntersectionType::Intersect && settings->culling.enableFrustumCulling && settings->culling.enableModelFrustumCulling) {
                visibility = CollisionTester::IsInFrustumIntersectionType(globalWorldCollider, frustum);
                if (visibility == IntersectionType::Outside) return;
            }

            bool parentFullyInside = (visibility == IntersectionType::Inside);

            std::span<const uint32_t> overrides;
            if (overridePool && overridePool->Has(entity)) {
                overrides = overridePool->Get(entity).materials;
            }

            for (uint32_t m = 0; m < meshCount; ++m)
            {
                bool isVisible = true;
                GpuMeshCollider worldCollider;

                if (meshCount > 1)
                {
                    GpuMeshCollider localCollider;

                    if (hasAnimation) {
                        uint32_t frameOffset = animFrameIndex * animResource->cpuData.descriptor.globalMeshCount;
                        localCollider = animResource->cpuData.frameMeshColliders[frameOffset + m];
                    }
                    else {
                        localCollider = resource->cpuData.meshColliders[m];
                    }

                    worldCollider = MeshUtils::TransformCollider(localCollider, transform);

                    if (!parentFullyInside && settings->culling.enableFrustumCulling && settings->culling.enableMeshFrustumCulling)
                        isVisible = CollisionTester::IsInFrustum(worldCollider, frustum);
                }
                else {
                    worldCollider = globalWorldCollider;
                }

                if (isVisible)
                {
                    float screenSizePixels = CollisionTester::CalculateSphereScreenSize(
                        worldCollider.center, worldCollider.radius,
                        cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                    if (screenSizePixels < 1.0f) 
                        continue;

                    uint32_t lod = CollisionTester::CalculateLodFromScreenSize(screenSizePixels);

                    uint32_t allocIndex = modelAlloc.meshAllocationOffset + (m * 4) + lod;
                    const auto& meshAlloc = drawData->Models.meshAllocations[allocIndex];

                    uint32_t matIdx = resource->cpuData.meshMaterialIndices[m];
                    if (!overrides.empty() && m < overrides.size() && overrides[m] != UINT32_MAX) {
                        matIdx = overrides[m];
                    }

                    MaterialRenderType matType = (matIdx < matTypeSnapshot.size()) ? matTypeSnapshot[matIdx] : MaterialRenderType::Opaque1Sided;

                    if constexpr (ENABLE_DEBUG_LOGGING) {
                        std::string name = "Unknown";
                        if (tagPool && tagPool->Has(entity)) name = tagPool->Get(entity).name;
                        Info("[FrustumCulling] Visible in camera: Entity {} ({}) | First visible sub-mesh MatType: {}", (uint32_t)entity, name, (uint32_t)matType);
                    }

                    if (meshAlloc.activeTypes[matType])
                    {
                        uint32_t slotIndex = 0;
                        uint32_t indirectIdx = meshAlloc.indirectIndices[matType];

                        if (meshAlloc.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET)
                        {
                            std::atomic_ref<uint32_t> countRef(drawData->Models.paddedMeshletCounts[indirectIdx * 16]);
                            slotIndex = countRef.fetch_add(1, std::memory_order_relaxed);
                        }
                        else
                        {
                            std::atomic_ref<uint32_t> countRef(drawData->Models.paddedTraditionalCounts[indirectIdx * 16]);
                            slotIndex = countRef.fetch_add(1, std::memory_order_relaxed);
                        }

                        uint32_t bufferIndex = meshAlloc.instanceOffsets[matType] + slotIndex;
                        if (bufferIndex < drawData->Models.instances.Size()) {
                            uint32_t payload = static_cast<uint32_t>(entity);

                            if (parentFullyInside) {
                                payload |= (1u << 31);
                            }
                            else {
                                payload &= ~(1u << 31);
                            }

                            drawData->Models.instances[bufferIndex] = payload;
                        }
                    }
                }      
            }
        };

        const auto& staticEntities = transformPool->GetStorage().GetStaticEntities();
        const auto& dynamicEntities = transformPool->GetStorage().GetDynamicEntities();
        const auto& streamEntities = transformPool->GetStorage().GetStreamEntities();

        std::optional<tf::Task> staticTask;
        auto chunkGroup = &drawData->Chunks;

        if (settings->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh && chunkGroup->chunkCounter.load(std::memory_order_relaxed) > 0)
        {
            chunkGroup->visibleChunkCount.store(0, std::memory_order_relaxed);

            uint32_t activeChunks = chunkGroup->chunkCounter.load(std::memory_order_relaxed);

            staticTask = this->ForEachIndex(uint32_t(0), activeChunks, uint32_t(1), subflow, "Update Static Chunks",
                [settings, chunkGroup, staticEntities, cullFunc, &cameraComp](uint32_t chunkIdx) {
                    const auto& chunk = chunkGroup->chunks[chunkIdx];

                    IntersectionType visibility = IntersectionType::Intersect;

                    if (settings->culling.enableFrustumCulling && settings->culling.enableChunkFrustumCulling)
                    {
                        visibility = CollisionTester::TestAabbFrustumIntersectionType(chunk.minBounds, chunk.maxBounds, cameraComp.frustum);
                        if (visibility == IntersectionType::Outside) return;
                    }

                    uint32_t slot = chunkGroup->visibleChunkCount.fetch_add(1, std::memory_order_relaxed);
                    uint32_t packedId = chunkIdx;
                    if (visibility == IntersectionType::Inside) packedId |= (1u << 31);
                    chunkGroup->visibleChunkIds[slot] = packedId;

                    for (uint32_t i = 0; i < chunk.entityCount; ++i) {
                        EntityID entity = staticEntities[chunk.firstEntityIndex + i];
                        cullFunc(entity, visibility);
                    }
                });
        }
        else
        {
            staticTask = this->ForEach(staticEntities, subflow, "Update Static",
                [cullFunc](EntityID entity) { cullFunc(entity, IntersectionType::Intersect); });
        }

        auto dynamicTask = this->ForEach(dynamicEntities, subflow, "Update Dynamic",
            [cullFunc](EntityID entity) { cullFunc(entity, IntersectionType::Intersect); });
        auto streamTask = this->ForEach(streamEntities, subflow, "Update Stream",
            [cullFunc](EntityID entity) { cullFunc(entity, IntersectionType::Intersect); });

        if (staticTask) initTask.precede(*staticTask);
        if (dynamicTask) initTask.precede(*dynamicTask);
        if (streamTask) initTask.precede(*streamTask);
    }

    void ModelFrustumCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            bool needsCommandUpload = (settings->culling.geometryCullingDevice == CullingDeviceType::CPU) || (drawData->syncFramesRemaining.load(std::memory_order_relaxed) > 0);

            if (settings->culling.geometryCullingDevice == CullingDeviceType::CPU)
            {
                for (uint32_t i = 0; i < drawData->Models.activeTraditionalCount; ++i) {
                    drawData->Models.traditionalCmds[i].instanceCount = drawData->Models.paddedTraditionalCounts[i * 16];
                }

                for (uint32_t i = 0; i < drawData->Models.activeMeshletCount; ++i) {
                    drawData->Models.meshletCmds[i].groupCountX = drawData->Models.paddedMeshletCounts[i * 16];
                }

                size_t instanceSize = drawData->Models.totalAllocatedInstances * sizeof(uint32_t);
                if (instanceSize > 0) {
                    drawData->Models.instanceBuffer.Write(frameIndex, drawData->Models.instances.Data(), instanceSize, 0);
                }

                if (settings->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh)
                {
                    uint32_t visibleCount = drawData->Chunks.visibleChunkCount.load(std::memory_order_relaxed);
                    VkDrawIndirectCommand cmd = drawData->Chunks.wireframeCmdTemplate;
                    cmd.instanceCount = visibleCount;

                    drawData->Chunks.chunkIndirectDispatchBuffer.Write(frameIndex, &cmd, sizeof(VkDrawIndirectCommand), 0);
                    drawData->Chunks.chunkVisibilityBuffer.Write(frameIndex, drawData->Chunks.visibleChunkIds.data(), visibleCount * sizeof(uint32_t), 0);
                }
            }

            if (needsCommandUpload)
            {
                size_t tradSize = drawData->Models.activeTraditionalCount * sizeof(VkDrawIndirectCommand);
                if (tradSize > 0) {
                    drawData->Models.indirectBuffer.Write(frameIndex, drawData->Models.traditionalCmds.Data(), tradSize, 0);
                }

                size_t meshletSize = drawData->Models.activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
                if (meshletSize > 0) {
                    size_t meshletGpuOffset = tradSize;
                    drawData->Models.indirectBuffer.Write(frameIndex, drawData->Models.meshletCmds.Data(), meshletSize, meshletGpuOffset);
                }

				auto totalCommandCount = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
                if (totalCommandCount > 0)
                {
                    drawData->Debug.modelAabbIndirectBuffer.Write(frameIndex, drawData->Debug.modelAabbCmds.Data(), totalCommandCount * sizeof(VkDrawIndirectCommand), 0);
                    drawData->Debug.modelSphereIndirectBuffer.Write(frameIndex, drawData->Debug.modelSphereCmds.Data(), totalCommandCount * sizeof(VkDrawIndirectCommand), 0);
                }
            }
            });
    }
}