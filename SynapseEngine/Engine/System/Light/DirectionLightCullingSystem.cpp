#include "DirectionLightCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/DirectionLightComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/System/Light/DirectionLightSystem.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/Scene/BufferNames.h"
#include <atomic>

namespace Syn
{
    std::vector<TypeID> DirectionLightCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<DirectionLightSystem>::ID,
            TypeInfo<CameraSystem>::ID
        };
    }

    std::vector<TypeID> DirectionLightCullingSystem::GetWriteDependencies() const {
        return {
            TypeInfo<DirectionLightCullingSystem>::ID
        };
    }

    void DirectionLightCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<DirectionLightComponent>();

        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!pool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        size_t maxLights = pool->Size();

        tf::Task initTask = this->EmplaceTask(subflow, "Init DirLight Culling", [maxLights, drawData]() {
            drawData->directionLightCmdTemplate.instanceCount = 0;
            if (drawData->directionLightCpuInstanceBuffer.size() < maxLights) {
                drawData->directionLightCpuInstanceBuffer.resize(maxLights);
            }
            });

        if (drawData->useGpuCulling) {
            return;
        }

        auto cullFunc = [pool, drawData](EntityID entity) {
            const auto& lightComp = pool->Get(entity);

            // if (!lightComp.enabled) return;

            std::atomic_ref<uint32_t> countRef(drawData->directionLightCmdTemplate.instanceCount);
            uint32_t slot = countRef.fetch_add(1, std::memory_order_relaxed);

            if (slot < drawData->directionLightCpuInstanceBuffer.size()) {
                drawData->directionLightCpuInstanceBuffer[slot] = entity;
            }
            };

        auto streamTask = this->ForEach(pool->GetStorage().GetStreamEntities(), subflow, "Cull Stream DirLights", cullFunc);
        auto dynTask = this->ForEach(pool->GetStorage().GetDynamicEntities(), subflow, "Cull Dynamic DirLights", cullFunc);
        auto statTask = this->ForEach(pool->GetStorage().GetStaticEntities(), subflow, "Cull Static DirLights", cullFunc);

        if (streamTask) initTask.precede(*streamTask);
        if (dynTask) initTask.precede(*dynTask);
        if (statTask) initTask.precede(*statTask);
    }

    void DirectionLightCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto bufferManager = scene->GetComponentBufferManager();
            auto drawData = scene->GetSceneDrawData();
            uint32_t count = drawData->directionLightCmdTemplate.instanceCount;

            if (!drawData->useGpuCulling) {
                auto instanceBufferView = bufferManager->GetComponentBuffer(BufferNames::DirectionLightVisibleData, frameIndex);
                if (count > 0 && instanceBufferView.buffer) {
                    instanceBufferView.buffer->Write(drawData->directionLightCpuInstanceBuffer.data(), count * sizeof(uint32_t), 0);
                }
            }

            drawData->directionLightIndirectCommandBuffers[frameIndex]->Write(&drawData->directionLightCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            });
    }
}