// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "SpotLightCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "SpotLightSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include <atomic>

#include "Engine/Component/Core/TagComponent.h"
#include "Engine/System/Core/TagSystem.h"

namespace Syn
{
    std::vector<TypeID> SpotLightCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<SpotLightSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<TagSystem>::ID
        };
    }

    std::vector<TypeID> SpotLightCullingSystem::GetWriteDependencies() const {
        return {
            TypeInfo<SpotLightCullingSystem>::ID
        };
    }

    void SpotLightCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto settings = scene->GetSettings();
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<SpotLightComponent>();
        auto shadowPool = registry->GetPool<SpotLightShadowComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto tagPool = registry->GetPool<TagComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!pool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        size_t maxLights = pool->Size();

        tf::Task initTask = this->EmplaceTask(subflow, "Init Spot Light Culling", [this, maxLights, drawData]() {
            drawData->SpotLights.cmdTemplate.instanceCount = 0;
            if (drawData->SpotLights.instances.Size() < maxLights) {
                drawData->SpotLights.instances.Resize(maxLights);
            }

            drawData->SpotLightShadow.visibleLightCount = 0;
            if (drawData->SpotLightShadow.visibleLights.Size() < maxLights) {
                drawData->SpotLightShadow.visibleLights.Resize(maxLights);
            }
            });

        if (settings->culling.spotLightCullingDevice == CullingDeviceType::GPU) {
            return;
        }

        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto cullFunc = [this, settings, pool, cameraComp, drawData, screenRes, shadowPool, tagPool](EntityID entity) {
            if (tagPool && tagPool->Has(entity)) {
                if (!tagPool->Get(entity).globalEnabled) {
                    return;
                }
            }
            
            const auto& lightComp = pool->Get(entity);

            bool visibility = true;

            if (settings->culling.enableFrustumCulling && settings->culling.enableSpotLightFrustumCulling)
                visibility = CollisionTester::IsInFrustum(lightComp.sphereCollider.center, lightComp.sphereCollider.radius, lightComp.aabbCollider.min, lightComp.aabbCollider.max, cameraComp.frustum);

            if (visibility)
            {
                float screenSize = CollisionTester::CalculateSphereScreenSize(
                    lightComp.sphereCollider.center, lightComp.sphereCollider.radius,
                    cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                if (screenSize >= 1.0f)
                {
                    std::atomic_ref<uint32_t> countRef(drawData->SpotLights.cmdTemplate.instanceCount);
                    uint32_t slot = countRef.fetch_add(1, std::memory_order_relaxed);

                    if (slot < drawData->SpotLights.instances.Size()) {
                        drawData->SpotLights.instances[slot] = entity;
                    }

                    if (lightComp.useShadow && shadowPool && shadowPool->Has(entity)) {
                        std::atomic_ref<uint32_t> shadowCountRef(drawData->SpotLightShadow.visibleLightCount);
                        uint32_t shadowSlot = shadowCountRef.fetch_add(1, std::memory_order_relaxed);

                        if (shadowSlot < drawData->SpotLightShadow.visibleLights.Size()) {
                            drawData->SpotLightShadow.visibleLights[shadowSlot] = entity;
                        }
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

    void SpotLightCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            auto bufferManager = scene->GetComponentBufferManager();
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            uint32_t count = drawData->SpotLights.cmdTemplate.instanceCount;
            uint32_t shadowCount = drawData->SpotLightShadow.visibleLightCount;

            if (settings->culling.spotLightCullingDevice == CullingDeviceType::CPU) {
                auto instanceBufferView = bufferManager->GetComponentBuffer(BufferNames::SpotLightVisibleData, frameIndex);
                if (count > 0 && instanceBufferView.buffer) {
                    instanceBufferView.buffer->Write(drawData->SpotLights.instances.Data(), count * sizeof(uint32_t), 0);
                }

                auto shadowBufferView = bufferManager->GetComponentBuffer(BufferNames::SpotLightShadowVisibleData, frameIndex);
                if (shadowCount > 0 && shadowBufferView.buffer) {
                    shadowBufferView.buffer->Write(drawData->SpotLightShadow.visibleLights.Data(), shadowCount * sizeof(uint32_t), 0);
                }

                drawData->SpotLightShadow.visibleCountDispatchBuffer.Write(frameIndex, &drawData->SpotLightShadow.visibleLightCount, sizeof(uint32_t), 0);
            }

            drawData->SpotLights.indirectBuffer.Write(frameIndex , &drawData->SpotLights.cmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            
            });
    }
}