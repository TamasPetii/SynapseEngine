#include "SpotLightFrustumCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/SpotLightComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "SpotLightSystem.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include <atomic>

namespace Syn
{
    std::vector<TypeID> SpotLightFrustumCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<SpotLightSystem>::ID,
            TypeInfo<CameraSystem>::ID
        };
    }

    std::vector<TypeID> SpotLightFrustumCullingSystem::GetWriteDependencies() const {
        return {
            TypeInfo<SpotLightFrustumCullingSystem>::ID
        };
    }

    void SpotLightFrustumCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto settings = scene->GetSettings();
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<SpotLightComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!pool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        size_t maxLights = pool->Size();

        tf::Task initTask = this->EmplaceTask(subflow, "Init Spot Light Culling", [this, maxLights, drawData]() {
            drawData->spotLightCmdTemplate.instanceCount = 0;
            if (drawData->spotLightCpuInstanceBuffer.size() < maxLights) {
                drawData->spotLightCpuInstanceBuffer.resize(maxLights);
            }
            });

        if (settings->enableGpuCulling) {
            return;
        }

        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto cullFunc = [this, pool, cameraComp, drawData, screenRes](EntityID entity) {
            const auto& lightComp = pool->Get(entity);

            if (CollisionTester::IsInFrustum(lightComp.sphereCollider.center, lightComp.sphereCollider.radius, lightComp.aabbCollider.min, lightComp.aabbCollider.max, cameraComp.frustum))
            {
                float screenSize = CollisionTester::CalculateSphereScreenSize(
                    lightComp.sphereCollider.center, lightComp.sphereCollider.radius,
                    cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                if (screenSize >= 1.0f)
                {
                    std::atomic_ref<uint32_t> countRef(drawData->spotLightCmdTemplate.instanceCount);
                    uint32_t slot = countRef.fetch_add(1, std::memory_order_relaxed);

                    if (slot < drawData->spotLightCpuInstanceBuffer.size()) {
                        drawData->spotLightCpuInstanceBuffer[slot] = entity;
                    }
                }
            }
            };

        auto streamTask = this->ForEach(pool->GetStorage().GetStreamEntities(), subflow, "Cull Stream Spot Lights", cullFunc);
        auto dynTask = this->ForEach(pool->GetStorage().GetDynamicEntities(), subflow, "Cull Dynamic Spot Lights", cullFunc);
        auto statTask = this->ForEach(pool->GetStorage().GetStaticEntities(), subflow, "Cull Static Spot Lights", cullFunc);

        if (streamTask) initTask.precede(*streamTask);
        if (dynTask) initTask.precede(*dynTask);
        if (statTask) initTask.precede(*statTask);
    }

    void SpotLightFrustumCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            auto bufferManager = scene->GetComponentBufferManager();
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();
            uint32_t count = drawData->spotLightCmdTemplate.instanceCount;

            if (!settings->enableGpuCulling) {
                auto instanceBufferView = bufferManager->GetComponentBuffer(BufferNames::SpotLightVisibleData, frameIndex);
                if (count > 0 && instanceBufferView.buffer) {
                    instanceBufferView.buffer->Write(drawData->spotLightCpuInstanceBuffer.data(), count * sizeof(uint32_t), 0);
                }
            }

            drawData->spotLightIndirectCommandBuffers[frameIndex]->Write(&drawData->spotLightCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            });
    }
}