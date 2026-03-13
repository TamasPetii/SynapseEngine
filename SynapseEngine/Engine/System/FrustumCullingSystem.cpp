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

#include "Engine/Mesh/Utils/MeshUtils.h"
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

        auto cullFunc = [drawData, modelPool, transformPool, modelSnapshot, cameraComp](EntityID entity) {   
            std::span<const FrustumFace> frustum = cameraComp.frustum;
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

            //Frustum culling on the full model!
            const auto& globalLocalCollider = resource->gpuData.globalCollider;
            GpuMeshCollider globalWorldCollider = MeshUtils::TransformCollider(globalLocalCollider, transform);

            if (!CollisionTester::IsInFrustum(globalWorldCollider, frustum))
                return;

            for (uint32_t m = 0; m < meshCount; ++m)
            {
                bool isVisible = true;

                float distance = glm::length(cameraComp.position - globalWorldCollider.center);
                if (meshCount > 1)
                {
                    //Frustum culling on meshes
                    const auto& localCollider = resource->gpuData.indexedData.meshColliders[m];
                    GpuMeshCollider worldCollider = MeshUtils::TransformCollider(localCollider, transform);
                    isVisible = CollisionTester::IsInFrustum(worldCollider, frustum);
                    distance = glm::length(cameraComp.position - worldCollider.center);
                }

                if (isVisible)
                {
                    //Todo: Project Sphere to screne, and lod has to calculated according to it!

                    uint32_t lod;
                    if (distance < 50.0f) lod = 0;
                    else if (distance < 120.0f) lod = 1;
                    else if (distance < 300.0f) lod = 2;
                    else lod = 3;

                    uint32_t allocIndex = modelAlloc.meshAllocationOffset + (m * 4) + lod;
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