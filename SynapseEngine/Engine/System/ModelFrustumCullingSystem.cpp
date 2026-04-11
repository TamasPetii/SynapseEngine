#include "ModelFrustumCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/System/ModelSystem.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/System/AnimationSystem.h"
#include "Engine/System/MaterialSystem.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Component/MaterialOverrideComponent.h"

#include "Engine/Mesh/Utils/MeshUtils.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include <glm/gtc/matrix_access.hpp>
#include <span>
#include <atomic>

namespace Syn
{
    std::vector<TypeID> ModelFrustumCullingSystem::GetReadDependencies() const {
        return { 
            TypeInfo<ModelSystem>::ID,
            TypeInfo<TransformSystem>::ID,
            TypeInfo<RenderSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<AnimationSystem>::ID,
            TypeInfo<MaterialSystem>::ID
        };
    }

    void ModelFrustumCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto settings = scene->GetSettings();
        auto drawData = scene->GetSceneDrawData();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto matTypeSnapshot = materialManager->GetRenderTypeSnapshot();
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

        if (settings->enableGpuCulling) {
            return;
        }

        auto modelManager = ServiceLocator::GetModelManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto animPool = registry->GetPool<AnimationComponent>();

        EntityID cameraEntity = scene->GetSceneCameraEntity();
        if (!modelPool || !transformPool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        auto modelSnapshot = modelManager->GetResourceSnapshot();
        auto animSnapshot = animationManager->GetResourceSnapshot();

        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto cullFunc = [drawData, modelPool, transformPool, modelSnapshot, cameraComp, animPool, animSnapshot, matTypeSnapshot, overridePool, screenRes](EntityID entity) {
            const FrustumCollider& frustum = cameraComp.frustum;
            const auto& modelComp = modelPool->Get(entity);
            const auto& transformComp = transformPool->Get(entity);

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

            GpuMeshCollider globalLocalCollider = resource->gpuData.globalCollider;

            if (hasAnimation) {
                globalLocalCollider = animResource->gpuData.frameGlobalColliders[animFrameIndex];
            }

            GpuMeshCollider globalWorldCollider = MeshUtils::TransformCollider(globalLocalCollider, transform);

            IntersectionType visibility = CollisionTester::IsInFrustumIntersectionType(globalWorldCollider, frustum);
            if (visibility == IntersectionType::Outside)
                return;

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
                        uint32_t frameOffset = animFrameIndex * animResource->gpuData.descriptor.globalMeshCount;
                        localCollider = animResource->gpuData.frameMeshColliders[frameOffset + m];
                    }
                    else {
                        localCollider = resource->gpuData.indexedData.meshColliders[m];
                    }

                    worldCollider = MeshUtils::TransformCollider(localCollider, transform);

                    if(!parentFullyInside)
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

                    uint32_t matIdx = resource->meshMaterialIndices[m];
                    if (!overrides.empty() && m < overrides.size() && overrides[m] != UINT32_MAX) {
                        matIdx = overrides[m];
                    }

                    MaterialRenderType matType = (matIdx < matTypeSnapshot.size()) ? matTypeSnapshot[matIdx] : MaterialRenderType::Opaque1Sided;

                    if (meshAlloc.activeTypes[matType])
                    {
                        uint32_t slotIndex = 0;
                        uint32_t indirectIdx = meshAlloc.indirectIndices[matType];

                        if (meshAlloc.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET)
                        {
                            // Minden számláló 16 uint32_t-re (64 bájtra) van egymástól!
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

        const auto& staticEntities = modelPool->GetStorage().GetStaticEntities();
        const auto& dynamicEntities = modelPool->GetStorage().GetDynamicEntities();
        const auto& streamEntities = modelPool->GetStorage().GetStreamEntities();

        auto staticTask = this->ForEach(staticEntities, subflow, "Update Static", cullFunc);
        auto dynamicTask = this->ForEach(dynamicEntities, subflow, "Update Dynamic", cullFunc);
        auto streamTask = this->ForEach(streamEntities, subflow, "Update Stream", cullFunc);

        if (staticTask) initTask.precede(*staticTask);
        if (dynamicTask) initTask.precede(*dynamicTask);
        if (streamTask) initTask.precede(*streamTask);
    }

    void ModelFrustumCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            bool needsCommandUpload = (!settings->enableGpuCulling) || (drawData->syncFramesRemaining.load(std::memory_order_relaxed) > 0);

            if (!settings->enableGpuCulling)
            {
                for (uint32_t i = 0; i < drawData->Models.activeTraditionalCount; ++i) {
                    drawData->Models.traditionalCmds[i].instanceCount = drawData->Models.paddedTraditionalCounts[i * 16];
                }

                for (uint32_t i = 0; i < drawData->Models.activeMeshletCount; ++i) {
                    drawData->Models.meshletCmds[i].groupCountX = drawData->Models.paddedMeshletCounts[i * 16];
                }

                size_t instanceSize = drawData->Models.totalAllocatedInstances * sizeof(uint32_t);
                if (instanceSize > 0) {
                    if (auto mappedInstance = drawData->Models.instanceBuffer.GetMapped(frameIndex)) {
                        mappedInstance->Write(drawData->Models.instances.Data(), instanceSize, 0);
                    }
                }
            }

            if (needsCommandUpload)
            {
                if (auto mappedIndirect = drawData->Models.indirectBuffer.GetMapped(frameIndex)) {
                    size_t tradSize = drawData->Models.activeTraditionalCount * sizeof(VkDrawIndirectCommand);
                    if (tradSize > 0) {
                        mappedIndirect->Write(drawData->Models.traditionalCmds.Data(), tradSize, 0);
                    }

                    size_t meshletSize = drawData->Models.activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
                    if (meshletSize > 0) {
                        size_t meshletGpuOffset = tradSize;
                        mappedIndirect->Write(drawData->Models.meshletCmds.Data(), meshletSize, meshletGpuOffset);
                    }
                }

                if (drawData->Models.activeMeshletCount > 0)
                {
                    if (auto mappedAabb = drawData->Debug.modelAabbIndirectBuffer.GetMapped(frameIndex)) {
                        mappedAabb->Write(&drawData->Debug.modelAabbCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
                    }

                    if (auto mappedSphere = drawData->Debug.modelSphereIndirectBuffer.GetMapped(frameIndex)) {
                        mappedSphere->Write(&drawData->Debug.modelSphereCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
                    }
                }
            }
            });
    }
}