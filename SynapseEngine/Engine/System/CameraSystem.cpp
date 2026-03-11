#include "CameraSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/InputManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <taskflow/algorithm/for_each.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

namespace Syn
{
    void CameraSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto cameraPool = registry->GetPool<CameraComponent>();
        if (!cameraPool) return;

        auto inputManager = ServiceLocator::GetInputManager();

        auto processEntity = [cameraPool, inputManager, deltaTime](EntityID entity) {
            if (inputManager->IsKeyHeld(KEY_W) || inputManager->IsKeyHeld(KEY_S) || inputManager->IsKeyHeld(KEY_A) || inputManager->IsKeyHeld(KEY_D) || inputManager->IsButtonHeld(BUTTON_RIGHT))
            {
                cameraPool->SetBit<UPDATE_BIT>(entity);
            }

            if (cameraPool->IsBitSet<UPDATE_BIT>(entity))
            {
                auto& cameraComponent = cameraPool->Get(entity);

                float forward = 0;
                float sideways = 0;

                if (inputManager->IsKeyHeld(KEY_W)) forward = 1;
                if (inputManager->IsKeyHeld(KEY_S)) forward = -1;
                if (inputManager->IsKeyHeld(KEY_D)) sideways = 1;
                if (inputManager->IsKeyHeld(KEY_A)) sideways = -1;

                if (inputManager->IsButtonHeld(BUTTON_RIGHT))
                {
                    auto deltaPos = inputManager->GetMouseDelta();
                    cameraComponent.yaw += cameraComponent.sensitivity * static_cast<float>(deltaPos.first);
                    cameraComponent.pitch += cameraComponent.sensitivity * -1.0f * static_cast<float>(deltaPos.second);
                    cameraComponent.pitch = glm::clamp<float>(cameraComponent.pitch, -89.f, 89.f);
                }

                glm::vec3 direction{
                    glm::cos(glm::radians(cameraComponent.yaw)) * glm::cos(glm::radians(cameraComponent.pitch)),
                    glm::sin(glm::radians(cameraComponent.pitch)),
                    glm::sin(glm::radians(cameraComponent.yaw)) * glm::cos(glm::radians(cameraComponent.pitch))
                };

                glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

                cameraComponent.direction = glm::normalize(direction);
                cameraComponent.right = glm::normalize(glm::cross(cameraComponent.direction, worldUp));
                cameraComponent.up = glm::normalize(glm::cross(cameraComponent.right, cameraComponent.direction));
                cameraComponent.position += (forward * cameraComponent.direction + sideways * cameraComponent.right) * cameraComponent.speed * deltaTime;
                cameraComponent.target = cameraComponent.position + cameraComponent.direction;

                cameraComponent.view = glm::lookAt(cameraComponent.position, cameraComponent.target, worldUp);
                cameraComponent.viewInv = glm::inverse(cameraComponent.view);

                cameraComponent.proj = glm::perspective(glm::radians(cameraComponent.fov), cameraComponent.width / cameraComponent.height, cameraComponent.nearPlane, cameraComponent.farPlane);
                cameraComponent.projInv = glm::inverse(cameraComponent.proj);

                cameraComponent.viewProj = cameraComponent.proj * cameraComponent.view;
                cameraComponent.viewProjInv = glm::inverse(cameraComponent.viewProj);

                cameraPool->SetBit<CHANGED_BIT>(entity);
                cameraComponent.version++;
            }
            };

        ParallelForEach(cameraPool, subflow, processEntity);
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

        auto cameraFrustumBuffer = componentBufferManager->GetComponentBuffer(BufferNames::CameraFrustumData, frameIndex);
        if (!cameraFrustumBuffer.buffer) return;
        auto cameraFrustumBufferHandler = static_cast<CameraFrustumGPU*>(cameraFrustumBuffer.buffer->Map());

        auto processUpload = [cameraPool, bufferHandler, cameraFrustumBufferHandler, componentBuffer, cameraFrustumBuffer](EntityID entity) {
            auto& cameraComponent = cameraPool->Get(entity);
            auto cameraIndex = cameraPool->GetMapping().Get(entity);

            if (componentBuffer.versions[cameraIndex] != cameraComponent.version)
            {
                componentBuffer.versions[cameraIndex] = cameraComponent.version;

                CameraComponent component = cameraComponent;

                glm::mat4 vulkanProj = component.proj;
                vulkanProj[1][1] *= -1;

                CameraComponentGPU cameraGPU{ component };
                cameraGPU.projVulkan = vulkanProj;
                cameraGPU.projVulkanInv = glm::inverse(cameraGPU.projVulkan);
                cameraGPU.viewProjVulkan = vulkanProj * component.view;
                cameraGPU.viewProjVulkanInv = glm::inverse(cameraGPU.viewProjVulkan);

                bufferHandler[cameraIndex] = cameraGPU;
            }

            /*
            if (cameraFrustumBuffer.versions[cameraIndex] != cameraComponent.version)
            {
                cameraFrustumBuffer.versions[cameraIndex] = cameraComponent.version;

                float fovY = glm::radians(cameraComponent.fov);
                float aspectRatio = cameraComponent.width / cameraComponent.height;
                float halfV = cameraComponent.farPlane * tanf(fovY * 0.5f);
                float halfH = halfV * aspectRatio;

                CameraFrustumGPU cameraFrustum;

                cameraFrustum.near = FrustumFace(cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.nearPlane);
                cameraFrustum.right = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.right * halfH), cameraComponent.up), cameraComponent.position);
                cameraFrustum.left = FrustumFace(-glm::cross(cameraComponent.up, glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.right * halfH)), cameraComponent.position);
                cameraFrustum.top = FrustumFace(-glm::cross(cameraComponent.right, glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.up * halfV)), cameraComponent.position);
                cameraFrustum.bottom = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.up * halfV), cameraComponent.right), cameraComponent.position);
                cameraFrustum.far = FrustumFace(-cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.farPlane);

                cameraFrustumBufferHandler[cameraIndex] = cameraFrustum;
            }
            */
            };

        ForEachStream(cameraPool, subflow, processUpload);

        if (uploadDynamic)
        {
            ForEachDynamic(cameraPool, subflow, processUpload);
            ForEachStatic(cameraPool, subflow, processUpload);
        }
    }
}