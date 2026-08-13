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

#include "PointLightCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "PointLightSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include <atomic>

#include "Engine/Component/Core/TagComponent.h"
#include "Engine/System/Core/TagSystem.h"

namespace Syn
{
    std::vector<TypeID> PointLightCullingSystem::GetReadDependencies() const {
        return { 
            TypeInfo<PointLightSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<TagSystem>::ID,
        };
    }

    std::vector<TypeID> PointLightCullingSystem::GetWriteDependencies() const {
        return { 
            TypeInfo<PointLightCullingSystem>::ID
        };
    }

    void PointLightCullingSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
		auto settings = scene->GetSettings();
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<PointLightComponent>();
        auto shadowPool = registry->GetPool<PointLightShadowComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto tagPool = registry->GetPool<TagComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!pool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);
        size_t maxLights = pool->Size();

        tf::Task initTask = this->EmplaceTask(subflow, "Init Point Light Culling", [this, maxLights, drawData]() {
            drawData->PointLights.cmdTemplate.instanceCount = 0;
            if (drawData->PointLights.instances.Size() < maxLights) {
                drawData->PointLights.instances.Resize(maxLights);
            }

            drawData->PointLightShadow.visibleLightCount = 0;
            if (drawData->PointLightShadow.visibleLights.Size() < maxLights) {
                drawData->PointLightShadow.visibleLights.Resize(maxLights);
            }
            });

        if (settings->culling.pointLightCullingDevice == CullingDeviceType::GPU) {
            return;
        }

        glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

        auto cullFunc = [this, settings, pool, shadowPool, cameraComp, drawData, screenRes, tagPool](EntityID entity) {
            if (tagPool && tagPool->Has(entity)) {
                if (!tagPool->Get(entity).globalEnabled) {
                    return;
                }
            }

            const auto& lightComp = pool->Get(entity);

            bool visibility = true;

            if (settings->culling.enableFrustumCulling && settings->culling.enablePointLightFrustumCulling)
                visibility = CollisionTester::TestSphereFrustum(lightComp.position, lightComp.radius, cameraComp.frustum);

            if (visibility)
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

                    if (lightComp.useShadow && shadowPool && shadowPool->Has(entity)) {
                        std::atomic_ref<uint32_t> shadowCountRef(drawData->PointLightShadow.visibleLightCount);
                        uint32_t shadowSlot = shadowCountRef.fetch_add(1, std::memory_order_relaxed);

                        if (shadowSlot < drawData->PointLightShadow.visibleLights.Size()) {
                            drawData->PointLightShadow.visibleLights[shadowSlot] = entity;
                        }
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

    void PointLightCullingSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            auto bufferManager = scene->GetComponentBufferManager();
            auto drawData = scene->GetSceneDrawData();
            auto settings = scene->GetSettings();

            uint32_t count = drawData->PointLights.cmdTemplate.instanceCount;
            uint32_t shadowCount = drawData->PointLightShadow.visibleLightCount;

            if (settings->culling.pointLightCullingDevice == CullingDeviceType::CPU) 
            {
                auto instanceBufferView = bufferManager->GetComponentBuffer(BufferNames::PointLightVisibleData, frameIndex);
                if (count > 0 && instanceBufferView.buffer) {
                    instanceBufferView.buffer->Write(drawData->PointLights.instances.Data(), count * sizeof(uint32_t), 0);
                }

                auto shadowBufferView = bufferManager->GetComponentBuffer(BufferNames::PointLightShadowVisibleData, frameIndex);
                if (shadowCount > 0 && shadowBufferView.buffer) {
                    shadowBufferView.buffer->Write(drawData->PointLightShadow.visibleLights.Data(), shadowCount * sizeof(uint32_t), 0);
                }

                drawData->PointLightShadow.visibleCountDispatchBuffer.Write(frameIndex, &drawData->PointLightShadow.visibleLightCount, sizeof(uint32_t), 0);
            }

            drawData->PointLights.indirectBuffer.Write(frameIndex, &drawData->PointLights.cmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            });
    }
}