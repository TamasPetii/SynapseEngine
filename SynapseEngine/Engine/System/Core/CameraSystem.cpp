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

#include "CameraSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/InputManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <taskflow/algorithm/for_each.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/System/Core/TransformSystem.h"

namespace Syn
{
    std::vector<TypeID> CameraSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<TransformSystem>::ID
        };
    }

    std::vector<TypeID> CameraSystem::GetWriteDependencies() const
    {
        return {
            TypeInfo<CameraSystem>::ID,
            TypeInfo<TransformSystem>::ID
        };
    }

    void CameraSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto cameraPool = registry->GetPool<CameraComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();

        if (!cameraPool || !transformPool) return;

        auto inputManager = ServiceLocator::Get<InputManager>();

        auto processEntity = [cameraPool, transformPool, inputManager, scene, deltaTime](EntityID entity) {
            if (!transformPool->Has(entity)) 
                return;

            bool useDebugCam = scene->GetSettings()->debug.useDebugCamera;
            bool enableInput = (useDebugCam && entity == scene->GetDebugCameraEntity()) || (!useDebugCam && entity == scene->GetSceneCameraEntity());

            auto& cameraComponent = cameraPool->Get(entity);
            auto& transformComponent = transformPool->Get(entity);

            float forward = 0;
            float sideways = 0;

            if (enableInput && inputManager->IsButtonHeld(BUTTON_RIGHT))
            {
                auto deltaPos = inputManager->GetMouseDelta();
                transformComponent.rotation.y += cameraComponent.sensitivity * static_cast<float>(deltaPos.first);
                transformComponent.rotation.x += cameraComponent.sensitivity * -1.0f * static_cast<float>(deltaPos.second);
                transformComponent.rotation.x = glm::clamp<float>(transformComponent.rotation.x, -89.f, 89.f);
            }

            glm::vec3 direction{
                glm::cos(glm::radians(transformComponent.rotation.y)) * glm::cos(glm::radians(transformComponent.rotation.x)),
                glm::sin(glm::radians(transformComponent.rotation.x)),
                glm::sin(glm::radians(transformComponent.rotation.y)) * glm::cos(glm::radians(transformComponent.rotation.x))
            };

            glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            cameraComponent.direction = glm::normalize(direction);
            cameraComponent.right = glm::normalize(glm::cross(cameraComponent.direction, worldUp));
            cameraComponent.up = glm::normalize(glm::cross(cameraComponent.right, cameraComponent.direction));

            if (cameraComponent.useOrbit)
            {
                if (enableInput)
                {
                    float sideways = 0;
                    if (inputManager->IsKeyHeld(KEY_D)) sideways = 1;
                    if (inputManager->IsKeyHeld(KEY_A)) sideways = -1;

                    cameraComponent.target += (sideways * cameraComponent.right) * cameraComponent.speed * deltaTime;

                    if (inputManager->IsKeyHeld(KEY_W)) cameraComponent.distance -= cameraComponent.speed * deltaTime;
                    if (inputManager->IsKeyHeld(KEY_S)) cameraComponent.distance += cameraComponent.speed * deltaTime;

                    cameraComponent.distance = glm::max(cameraComponent.distance, 0.1f);
                }

                cameraComponent.position = cameraComponent.target - cameraComponent.direction * cameraComponent.distance;
                transformComponent.translation = cameraComponent.position;
            }
            else
            {
                float forward = 0;
                float sideways = 0;

                if (enableInput && inputManager->IsKeyHeld(KEY_W)) forward = 1;
                if (enableInput && inputManager->IsKeyHeld(KEY_S)) forward = -1;
                if (enableInput && inputManager->IsKeyHeld(KEY_D)) sideways = 1;
                if (enableInput && inputManager->IsKeyHeld(KEY_A)) sideways = -1;

                transformComponent.translation += (forward * cameraComponent.direction + sideways * cameraComponent.right) * cameraComponent.speed * deltaTime;

                cameraComponent.position = transformComponent.translation;
                cameraComponent.target = cameraComponent.position + cameraComponent.direction;
            }

            cameraComponent.view = glm::lookAt(cameraComponent.position, cameraComponent.target, worldUp);
            cameraComponent.viewInv = glm::inverse(cameraComponent.view);

            if (cameraComponent.isOrthographic)
            {
                float aspectRatio = cameraComponent.width / cameraComponent.height;
                float halfH = cameraComponent.orthoSize * 0.5f;
                float halfW = halfH * aspectRatio;
                cameraComponent.proj = glm::ortho(-halfW, halfW, -halfH, halfH, -cameraComponent.farPlane, cameraComponent.farPlane);
            }
            else
            {
                cameraComponent.proj = glm::perspective(glm::radians(cameraComponent.fov), cameraComponent.width / cameraComponent.height, cameraComponent.nearPlane, cameraComponent.farPlane);
            }

            cameraComponent.projInv = glm::inverse(cameraComponent.proj);

            cameraComponent.viewProj = cameraComponent.proj * cameraComponent.view;
            cameraComponent.viewProjInv = glm::inverse(cameraComponent.viewProj);

            if (cameraComponent.isOrthographic)
            {
                float aspectRatio = cameraComponent.width / cameraComponent.height;
                float halfH = cameraComponent.orthoSize * 0.5f;
                float halfW = halfH * aspectRatio;

                cameraComponent.frustum.planes[0] = FrustumCollider::CreatePlane(
                    cameraComponent.direction,
                    cameraComponent.position + cameraComponent.direction * cameraComponent.nearPlane
                );
                cameraComponent.frustum.planes[1] = FrustumCollider::CreatePlane(
                    -cameraComponent.right,
                    cameraComponent.position + cameraComponent.right * halfW
                );
                cameraComponent.frustum.planes[2] = FrustumCollider::CreatePlane(
                    cameraComponent.right,
                    cameraComponent.position - cameraComponent.right * halfW
                );
                cameraComponent.frustum.planes[3] = FrustumCollider::CreatePlane(
                    -cameraComponent.up,
                    cameraComponent.position + cameraComponent.up * halfH
                );
                cameraComponent.frustum.planes[4] = FrustumCollider::CreatePlane(
                    cameraComponent.up,
                    cameraComponent.position - cameraComponent.up * halfH
                );
                cameraComponent.frustum.planes[5] = FrustumCollider::CreatePlane(
                    -cameraComponent.direction,
                    cameraComponent.position + cameraComponent.direction * cameraComponent.farPlane
                );
            }
            else
            {
                cameraComponent.frustum.Update(cameraComponent.viewProj);
            }

            cameraPool->SetBit<CHANGED_BIT>(entity);
            transformPool->SetBit<UPDATE_BIT>(entity);
            cameraComponent.version++;
        };

        ParallelForEach(cameraPool, subflow, SystemPhaseNames::Update, processEntity);
    }

    void CameraSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto cameraPool = registry->GetPool<CameraComponent>();
        if (!cameraPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::CameraData, frameIndex);
        if (!componentBuffer.buffer) return;
        auto bufferHandler = static_cast<CameraComponentGPU*>(componentBuffer.buffer->Map());

        auto visibleBuffer = componentBufferManager->GetComponentBuffer(BufferNames::CameraVisibleData, frameIndex);
        if (!visibleBuffer.buffer) return;
        auto visibleBufferHandler = static_cast<uint32_t*>(visibleBuffer.buffer->Map());

        auto processUpload = [cameraPool, bufferHandler, componentBuffer, visibleBuffer, visibleBufferHandler](EntityID entity) {
            auto& cameraComponent = cameraPool->Get(entity);
            auto cameraIndex = cameraPool->GetMapping().Get(entity);

            if (componentBuffer.versions[cameraIndex] != cameraComponent.version)
            {
                componentBuffer.versions[cameraIndex] = cameraComponent.version;
                bufferHandler[cameraIndex] = CameraComponentGPU(cameraComponent);
            }

            if (visibleBuffer.versions[cameraIndex] != cameraComponent.version)
            {
                visibleBuffer.versions[cameraIndex] = cameraComponent.version;
                visibleBufferHandler[cameraIndex] = entity;
            }
        };

        ForEachStream(cameraPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
            ForEachDynamic(cameraPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadStatic)
            ForEachStatic(cameraPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}