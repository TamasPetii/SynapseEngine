#include "FrustumCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/System/ModelSystem.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/Manager/ModelManager.h"
#include "Engine/System/CameraSystem.h"
#include <atomic>

#include "Engine/Collision/Tester/CollisionTester.h"
#include <glm/gtc/matrix_access.hpp>
#include <span>

namespace Syn
{
    std::vector<TypeID> FrustumCullingSystem::GetReadDependencies() const {
        return { TypeInfo<ModelSystem>::ID, TypeInfo<TransformSystem>::ID, TypeInfo<RenderSystem>::ID, TypeInfo<CameraSystem>::ID };
    }

    void FrustumCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto modelManager = ServiceLocator::GetModelManager();

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();

        EntityID cameraEntity = scene->GetSceneCameraEntity();
        if (!modelPool || !transformPool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        std::span<const FrustumFace> frustum = cameraComp.frustum;

        auto drawData = scene->GetSceneDrawData();
        auto modelSnapshot = modelManager->GetResourceSnapshot();

        tf::Task initTask = subflow.emplace([drawData]() {
            for (uint32_t i = 0; i < drawData->activeTraditionalCount; ++i) {
                drawData->traditionalCommands[i].instanceCount = 0;
            }
            for (uint32_t i = 0; i < drawData->activeMeshletCount; ++i) {
                drawData->meshletCommands[i].groupCountX = 0;
            }
            }).name("FrustumCulling Init");

        auto transformCollider = [](const GpuMeshCollider& local, const glm::mat4& transform, const glm::mat3& absTransform, float maxScale) {
            glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(local.center, 1.0f));
            float worldRadius = local.radius * maxScale;

            glm::vec3 localAabbCenter = (local.aabbMax + local.aabbMin) * 0.5f;
            glm::vec3 localAabbExtents = (local.aabbMax - local.aabbMin) * 0.5f;

            glm::vec3 worldAabbCenter = glm::vec3(transform * glm::vec4(localAabbCenter, 1.0f));
            glm::vec3 worldAabbExtents = absTransform * localAabbExtents;

            GpuMeshCollider world;
            world.center = worldCenter;
            world.radius = worldRadius;
            world.aabbMin = worldAabbCenter - worldAabbExtents;
            world.aabbMax = worldAabbCenter + worldAabbExtents;

            return world;
            };

        auto cullFunc = [drawData, modelPool, transformPool, modelSnapshot, frustum, transformCollider](EntityID entity) {
            const auto& modelComp = modelPool->Get(entity);
            const auto& transformComp = transformPool->Get(entity);

            if (modelComp.modelIndex == NULL_INDEX)
                return;

            if (modelComp.modelIndex >= drawData->modelAllocations.size())
                return;

            const auto& snapshotEntry = modelSnapshot[modelComp.modelIndex];
            if (snapshotEntry.resource == nullptr || snapshotEntry.state != ResourceState::Ready)
                return;

            const auto& modelAlloc = drawData->modelAllocations[modelComp.modelIndex];

            uint32_t meshCount = modelAlloc.meshAllocationCount / 4;
            auto resource = snapshotEntry.resource;

            const glm::mat4& transform = transformComp.transform;
            glm::mat3 rotMatrix = glm::mat3(transform);
            glm::mat3 absTransform(
                glm::abs(rotMatrix[0]),
                glm::abs(rotMatrix[1]),
                glm::abs(rotMatrix[2])
            );

            float scaleX = glm::length(absTransform[0]);
            float scaleY = glm::length(absTransform[1]);
            float scaleZ = glm::length(absTransform[2]);
            float maxScale = std::max({ scaleX, scaleY, scaleZ });

            //Frustum culling on the whole model!
            const auto& globalLocalCollider = resource->gpuData.globalCollider;
            GpuMeshCollider globalWorldCollider = transformCollider(globalLocalCollider, transform, absTransform, maxScale);
            if (!CollisionTester::IsInFrustum(globalWorldCollider, frustum))
                return;

            for (uint32_t m = 0; m < meshCount; ++m)
            {
                bool isVisible = true;

                if (meshCount > 1)
                {
                    //Frustum culling on meshes
                    const auto& localCollider = resource->gpuData.indexedData.meshColliders[m];
                    GpuMeshCollider worldCollider = transformCollider(localCollider, transform, absTransform, maxScale);
                    isVisible = CollisionTester::IsInFrustum(worldCollider, frustum);
                }

                if (isVisible)
                {
                    uint32_t calculatedLod = rand() % 4;

                    uint32_t allocIndex = modelAlloc.meshAllocationOffset + (m * 4) + calculatedLod;
                    const auto& meshAlloc = drawData->meshAllocations[allocIndex];

                    uint32_t slotIndex = 0;

                    if (meshAlloc.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET)
                    {
                        uint32_t localCmdIndex = meshAlloc.indirectIndex - SceneDrawData::MESHLET_OFFSET_START;
                        auto& cmd = drawData->meshletCommands[localCmdIndex];

                        std::atomic_ref<uint32_t> countRef(cmd.groupCountX);
                        slotIndex = countRef.fetch_add(1, std::memory_order_relaxed);
                    }
                    else
                    {
                        auto& cmd = drawData->traditionalCommands[meshAlloc.indirectIndex];

                        std::atomic_ref<uint32_t> countRef(cmd.instanceCount);
                        slotIndex = countRef.fetch_add(1, std::memory_order_relaxed);
                    }

                    uint32_t bufferIndex = meshAlloc.instanceOffset + slotIndex;
                    if (bufferIndex < drawData->cpuInstanceBuffer.size()) {
                        drawData->cpuInstanceBuffer[bufferIndex] = entity;
                    }
                }      
            }
        };

        const auto& staticEntities = modelPool->GetStorage().GetStaticEntities();
        const auto& dynamicEntities = modelPool->GetStorage().GetDynamicEntities();
        const auto& streamEntities = modelPool->GetStorage().GetStreamEntities();

        tf::Task staticTask = subflow.for_each(staticEntities.begin(), staticEntities.end(), cullFunc).name("Cull Static");
        tf::Task dynamicTask = subflow.for_each(dynamicEntities.begin(), dynamicEntities.end(), cullFunc).name("Cull Dynamic");
        tf::Task streamTask = subflow.for_each(streamEntities.begin(), streamEntities.end(), cullFunc).name("Cull Stream");

        initTask.precede(staticTask, dynamicTask, streamTask);
    }

    void FrustumCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        subflow.emplace([scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();

            //Ez nem kell gpu driven esetén a többi kell!
            size_t instanceSize = drawData->totalAllocatedInstances * sizeof(uint32_t);
            if (instanceSize > 0) {
                drawData->globalInstanceBuffers[frameIndex]->Write(drawData->cpuInstanceBuffer.data(), instanceSize, 0);
            }

            size_t tradSize = drawData->activeTraditionalCount * sizeof(VkDrawIndirectCommand);
            if (tradSize > 0)
                drawData->globalIndirectCommandBuffers[frameIndex]->Write(drawData->traditionalCommands.data(), tradSize, 0);

            size_t meshletSize = drawData->activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
            if (meshletSize > 0)
            {
                size_t meshletGpuOffset = SceneDrawData::MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
                drawData->globalIndirectCommandBuffers[frameIndex]->Write(drawData->meshletCommands.data(), meshletSize, meshletGpuOffset);
            }

            }).name("FrustumCulling GPU Upload");
    }
}