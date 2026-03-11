#include "FrustumCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/System/ModelSystem.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/System/RenderSystem.h"
#include <atomic>

namespace Syn
{
    std::vector<TypeID> FrustumCullingSystem::GetReadDependencies() const {
        return { TypeInfo<ModelSystem>::ID, TypeInfo<TransformSystem>::ID, TypeInfo<RenderSystem>::ID };
    }

    void FrustumCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        if (!modelPool || !transformPool) return;

        auto drawData = scene->GetSceneDrawData();

        tf::Task initTask = subflow.emplace([drawData]() {
            for (uint32_t i = 0; i < drawData->activeTraditionalCount; ++i) {
                drawData->traditionalCommands[i].instanceCount = 0;
            }
            for (uint32_t i = 0; i < drawData->activeMeshletCount; ++i) {
                drawData->meshletCommands[i].groupCountX = 0;
            }
            }).name("FrustumCulling Init");

        auto cullFunc = [drawData, modelPool](EntityID entity) {
            const auto& modelComp = modelPool->Get(entity);
            if (modelComp.modelIndex == NULL_INDEX) return;

            if (modelComp.modelIndex >= drawData->modelAllocations.size()) return;
            const auto& modelAlloc = drawData->modelAllocations[modelComp.modelIndex];

            bool isVisible = true;
            uint32_t calculatedLod = 0;

            if (isVisible)
            {
                uint32_t meshCount = modelAlloc.meshAllocationCount / 4;

                for (uint32_t m = 0; m < meshCount; ++m)
                {
                    uint32_t allocIndex = modelAlloc.meshAllocationOffset + (m * 4) + calculatedLod;
                    const auto& meshAlloc = drawData->meshAllocations[allocIndex];

                    uint32_t slotIndex = 0;

                    if (meshAlloc.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET)
                    {
                        uint32_t localCmdIndex = meshAlloc.indirectIndex - SceneDrawData::MESHLET_OFFSET_START;
                        auto& cmd = drawData->meshletCommands[localCmdIndex];

                        std::atomic_ref<uint32_t> countRef(cmd.groupCountX);

                        // Mesh shader esetén a groupCountX a feladatok száma. 
                        // TODO: Itt majd a meshletCount-ot kell hozzáadni (atomic fetch_add(meshletCount))
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