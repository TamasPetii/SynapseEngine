#include "CameraSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/InputManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <taskflow/algorithm/for_each.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "Engine/Component/TransformComponent.h"
#include "Engine/System/TransformSystem.h"

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

        auto inputManager = ServiceLocator::GetInputManager();

        auto processEntity = [cameraPool, transformPool, inputManager, scene, deltaTime](EntityID entity) {
            if (!transformPool->Has(entity)) 
                return;

            if (inputManager->IsKeyHeld(KEY_W) || inputManager->IsKeyHeld(KEY_S) || inputManager->IsKeyHeld(KEY_A) || inputManager->IsKeyHeld(KEY_D) || inputManager->IsButtonHeld(BUTTON_RIGHT))
            {
                //if (entity == scene->GetDebugCameraEntity())
                if (entity == scene->GetSceneCameraEntity())
                {
                    cameraPool->SetBit<UPDATE_BIT>(entity);
                    transformPool->SetBit<UPDATE_BIT>(entity);
                }
            }

            if (cameraPool->IsBitSet<UPDATE_BIT>(entity))
            {
                auto& cameraComponent = cameraPool->Get(entity);
                auto& transformComponent = transformPool->Get(entity);

                float forward = 0;
                float sideways = 0;

                if (inputManager->IsKeyHeld(KEY_W)) forward = 1;
                if (inputManager->IsKeyHeld(KEY_S)) forward = -1;
                if (inputManager->IsKeyHeld(KEY_D)) sideways = 1;
                if (inputManager->IsKeyHeld(KEY_A)) sideways = -1;

                if (inputManager->IsButtonHeld(BUTTON_RIGHT))
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

                transformComponent.translation += (forward * cameraComponent.direction + sideways * cameraComponent.right) * cameraComponent.speed * deltaTime;

                cameraComponent.position = transformComponent.translation;
                cameraComponent.target = cameraComponent.position + cameraComponent.direction;

                cameraComponent.view = glm::lookAt(cameraComponent.position, cameraComponent.target, worldUp);
                cameraComponent.viewInv = glm::inverse(cameraComponent.view);

                cameraComponent.proj = glm::perspective(glm::radians(cameraComponent.fov), cameraComponent.width / cameraComponent.height, cameraComponent.nearPlane, cameraComponent.farPlane);
                cameraComponent.projInv = glm::inverse(cameraComponent.proj);

                cameraComponent.viewProj = cameraComponent.proj * cameraComponent.view;
                cameraComponent.viewProjInv = glm::inverse(cameraComponent.viewProj);

                float fovY = glm::radians(cameraComponent.fov);
                float aspectRatio = cameraComponent.width / cameraComponent.height;
                float halfV = cameraComponent.farPlane * tanf(fovY * 0.5f);
                float halfH = halfV * aspectRatio;

                cameraComponent.frustum[0] = FrustumFace(cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.nearPlane);
                cameraComponent.frustum[1] = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.right * halfH), cameraComponent.up), cameraComponent.position);
                cameraComponent.frustum[2] = FrustumFace(-glm::cross(cameraComponent.up, glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.right * halfH)), cameraComponent.position);
                cameraComponent.frustum[3] = FrustumFace(-glm::cross(cameraComponent.right, glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.up * halfV)), cameraComponent.position);
                cameraComponent.frustum[4] = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.up * halfV), cameraComponent.right), cameraComponent.position);
                cameraComponent.frustum[5] = FrustumFace(-cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.farPlane);

                cameraPool->SetBit<CHANGED_BIT>(entity);
                cameraComponent.version++;
            }
            };

        ParallelForEach(cameraPool, subflow, SystemPhaseNames::Update, processEntity);
    }

    void CameraSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto cameraPool = registry->GetPool<CameraComponent>();
        if (!cameraPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::CameraData, frameIndex);
        if (!componentBuffer.buffer) return;
        auto bufferHandler = static_cast<CameraComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [cameraPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& cameraComponent = cameraPool->Get(entity);
            auto cameraIndex = cameraPool->GetMapping().Get(entity);

            if (componentBuffer.versions[cameraIndex] != cameraComponent.version)
            {
                componentBuffer.versions[cameraIndex] = cameraComponent.version;
                bufferHandler[cameraIndex] = CameraComponentGPU(cameraComponent);
            }
        };

        ForEachStream(cameraPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
        {
            ForEachDynamic(cameraPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
            ForEachStatic(cameraPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        }
    }
}