#include "PointLightFrustumCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/PointLightComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "PointLightSystem.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include <atomic>

namespace Syn
{
    std::vector<TypeID> PointLightFrustumCullingSystem::GetReadDependencies() const {
        return { 
            TypeInfo<PointLightSystem>::ID,
            TypeInfo<CameraSystem>::ID
        };
    }

    std::vector<TypeID> PointLightFrustumCullingSystem::GetWriteDependencies() const {
        return { 
            TypeInfo<PointLightFrustumCullingSystem>::ID
        };
    }

    void PointLightFrustumCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
		auto settings = scene->GetSettings();
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<PointLightComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!pool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        size_t maxLights = pool->Size();

        tf::Task initTask = this->EmplaceTask(subflow, "Init Light Culling", [this, maxLights, drawData]() {
            drawData->PointLights.cmdTemplate.instanceCount = 0;
            if (drawData->PointLights.instances.Size() < maxLights) {
                drawData->PointLights.instances.Resize(maxLights);
            }
            });

        if (settings->enableGpuCulling) {
            return;
        }

        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto cullFunc = [this, pool, cameraComp, drawData, screenRes](EntityID entity) {
            const auto& lightComp = pool->Get(entity);

            if (CollisionTester::TestSphereFrustum(lightComp.position, lightComp.radius, cameraComp.frustum))
            {
                float screenSize = CollisionTester::CalculateSphereScreenSize(
                    lightComp.position, lightComp.radius,
                    cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                if (screenSize >= 1.0f)
                {
                    std::atomic_ref<uint32_t> countRef(drawData->PointLights.cmdTemplate.instanceCount);
                    uint32_t slot = countRef.fetch_add(1, std::memory_order_relaxed);

                    if (slot < drawData->PointLights.instances.Size()) {
                        drawData->PointLights.instances[slot] = entity;
                    }
                }
            }
            };

        auto streamTask = this->ForEach(pool->GetStorage().GetStreamEntities(), subflow, "Cull Stream Lights", cullFunc);
        auto dynTask = this->ForEach(pool->GetStorage().GetDynamicEntities(), subflow, "Cull Dynamic Lights", cullFunc);
        auto statTask = this->ForEach(pool->GetStorage().GetStaticEntities(), subflow, "Cull Static Lights", cullFunc);

        if (streamTask) initTask.precede(*streamTask);
        if (dynTask) initTask.precede(*dynTask);
        if (statTask) initTask.precede(*statTask);
    }

    void PointLightFrustumCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            auto bufferManager = scene->GetComponentBufferManager();
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();
            uint32_t count = drawData->PointLights.cmdTemplate.instanceCount;

            if (!settings->enableGpuCulling) {
                auto instanceBufferView = bufferManager->GetComponentBuffer(BufferNames::PointLightVisibleData, frameIndex);
                if (count > 0 && instanceBufferView.buffer) {
                    instanceBufferView.buffer->Write(drawData->PointLights.instances.Data(), count * sizeof(uint32_t), 0);
                }
            }

            if (auto mapped = drawData->PointLights.indirectBuffer.GetMapped(frameIndex)) {
                mapped->Write(&drawData->PointLights.cmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            }
            });
    }
}