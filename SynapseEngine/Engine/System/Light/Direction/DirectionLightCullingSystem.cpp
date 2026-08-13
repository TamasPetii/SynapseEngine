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

#include "DirectionLightCullingSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/System/Light/Direction/DirectionLightSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/Scene/BufferNames.h"
#include <atomic>

#include "Engine/Component/Core/TagComponent.h"
#include "Engine/System/Core/TagSystem.h"

namespace Syn
{
    std::vector<TypeID> DirectionLightCullingSystem::GetReadDependencies() const {
        return {
            TypeInfo<DirectionLightSystem>::ID,
            TypeInfo<CameraSystem>::ID,
            TypeInfo<TagSystem>::ID
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

        auto tagPool = registry->GetPool<TagComponent>();
        auto pool = registry->GetPool<DirectionLightComponent>();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();

        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!pool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        size_t maxLights = pool->Size();

        tf::Task initTask = this->EmplaceTask(subflow, "Init DirLight Culling", [maxLights, drawData]() {
            drawData->DirectionLights.cmdTemplate.instanceCount = 0;
            if (drawData->DirectionLights.instances.Size() < maxLights) {
                drawData->DirectionLights.instances.Resize(maxLights);
            }

            drawData->DirectionLightShadow.visibleLightCount = 0;
            if (drawData->DirectionLightShadow.visibleLights.Size() < maxLights) {
                drawData->DirectionLightShadow.visibleLights.Resize(maxLights);
            }
            });

        auto cullFunc = [pool, drawData, shadowPool, tagPool](EntityID entity) {
            if (tagPool && tagPool->Has(entity)) {
                if (!tagPool->Get(entity).globalEnabled) {
                    return;
                }
            }
            
            const auto& lightComp = pool->Get(entity);

            std::atomic_ref<uint32_t> countRef(drawData->DirectionLights.cmdTemplate.instanceCount);
            uint32_t slot = countRef.fetch_add(1, std::memory_order_relaxed);

            if (slot < drawData->DirectionLights.instances.Size()) {
                drawData->DirectionLights.instances[slot] = entity;
            }

            if (shadowPool && shadowPool->Has(entity))
            {
                std::atomic_ref<uint32_t> shadowCountRef(drawData->DirectionLightShadow.visibleLightCount);
                uint32_t shadowSlot = shadowCountRef.fetch_add(1, std::memory_order_relaxed);

                if (shadowSlot < drawData->DirectionLightShadow.visibleLights.Size()) {
                    drawData->DirectionLightShadow.visibleLights[shadowSlot] = entity;
                }
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
            auto settings = scene->GetSettings();
            uint32_t count = drawData->DirectionLights.cmdTemplate.instanceCount;

            auto instanceBufferView = bufferManager->GetComponentBuffer(BufferNames::DirectionLightVisibleData, frameIndex);
            if (count > 0 && instanceBufferView.buffer) {
                instanceBufferView.buffer->Write(drawData->DirectionLights.instances.Data(), count * sizeof(uint32_t), 0);
            }

            auto visibleShadowBufferView = bufferManager->GetComponentBuffer(BufferNames::DirectionLightVisibleShadowData, frameIndex);
            if (count > 0 && visibleShadowBufferView.buffer) {
                visibleShadowBufferView.buffer->Write(drawData->DirectionLightShadow.visibleLights.Data(), count * sizeof(uint32_t), 0);
            }

            drawData->DirectionLights.indirectBuffer.Write(frameIndex , &drawData->DirectionLights.cmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            });
    }
}