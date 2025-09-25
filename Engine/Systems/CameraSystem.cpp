#include "CameraSystem.h"
#include "Engine/Managers/InputManager.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Physics/Collider/FrustumCollider.h"

void CameraSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto inputManager = InputManager::Instance();
	auto [transformPool, cameraPool] = registry->GetPools<TransformComponent, CameraComponent>();

	if (!transformPool || !cameraPool)
		return;

	std::for_each(std::execution::par, cameraPool->GetDenseIndices().begin(), cameraPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void
		{
			if (inputManager->IsKeyHeld(KEY_W) || inputManager->IsKeyHeld(KEY_S) || inputManager->IsKeyHeld(KEY_A) || inputManager->IsKeyHeld(KEY_D) || inputManager->IsButtonHeld(BUTTON_RIGHT))
				cameraPool->SetBit<UPDATE_BIT>(entity);

			if (cameraPool->IsBitSet<UPDATE_BIT>(entity))
			{
				auto& transformComponent = transformPool->GetData(entity);
				auto& cameraComponent = cameraPool->GetData(entity);

				float forward = 0;
				float sideways = 0;
				float distance = 0;

				if (inputManager->IsKeyHeld(KEY_W))
					forward = 1;
				if (inputManager->IsKeyHeld(KEY_S))
					forward = -1;
				if (inputManager->IsKeyHeld(KEY_D))
					sideways = 1;
				if (inputManager->IsKeyHeld(KEY_A))
					sideways = -1;
				if (inputManager->IsButtonHeld(BUTTON_RIGHT))
				{
					auto deltaPos = inputManager->GetMouseDelta();
					cameraComponent.yaw += cameraComponent.sensitivity * deltaPos.first;
					cameraComponent.pitch += cameraComponent.sensitivity * -1 * deltaPos.second;
					cameraComponent.pitch = glm::clamp<float>(cameraComponent.pitch, -89.f, 89.f);
				}

				glm::vec3 direction{
					glm::cos(glm::radians(cameraComponent.yaw)) * glm::cos(glm::radians(cameraComponent.pitch)),
					glm::sin(glm::radians(cameraComponent.pitch)),
					glm::sin(glm::radians(cameraComponent.yaw)) * glm::cos(glm::radians(cameraComponent.pitch))
				};

				glm::vec3 worldUp = GlobalConfig::World::up;

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
		}
	);
}

void CameraSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto cameraPool = registry->GetPool<CameraComponent>();

	if (!cameraPool)
		return;

	std::for_each(std::execution::par, cameraPool->GetDenseIndices().begin(), cameraPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if(cameraPool->IsBitSet<CHANGED_BIT>(entity))
				cameraPool->GetBitset(entity).reset();
		}
	);
}

void CameraSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto cameraPool = registry->GetPool<CameraComponent>();

	if (!cameraPool)
		return;

	auto componentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex);
	auto bufferHandler = static_cast<CameraComponentGPU*>(componentBuffer->buffer->GetHandler());

	auto cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex);
	auto cameraFrustumBufferHandler = static_cast<CameraFrustumGPU*>(cameraFrustumBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, cameraPool->GetDenseIndices().begin(), cameraPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& cameraComponent = cameraPool->GetData(entity);
			auto cameraIndex = cameraPool->GetDenseIndex(entity);

			if (componentBuffer->versions[cameraIndex] != cameraComponent.version)
			{
				componentBuffer->versions[cameraIndex] = cameraComponent.version;

				CameraComponent component = cameraComponent;
				component.proj[1][1] *= -1;
				component.projInv = glm::inverse(component.proj);
				component.viewProj = component.proj * component.view;
				component.viewProjInv = glm::inverse(component.viewProj);
				bufferHandler[cameraIndex] = CameraComponentGPU(component);
			}

			if (cameraFrustumBuffer->versions[cameraIndex] != cameraComponent.version)
			{
				cameraFrustumBuffer->versions[cameraIndex] = cameraComponent.version;

				float fovY = glm::radians(cameraComponent.fov);
				float aspectRatio = cameraComponent.width / cameraComponent.height;
				float halfV = cameraComponent.farPlane * tanf(fovY * 0.5f);
				float halfH = halfV * aspectRatio;

				//Todo: Function that handles this

				CameraFrustumGPU cameraFrustum;

				//NearFace
				cameraFrustum.near= FrustumFace(cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.nearPlane);

				//RightFace
				cameraFrustum.right = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.right * halfH), cameraComponent.up), cameraComponent.position);

				//LeftFace
				cameraFrustum.left = FrustumFace(-glm::cross(cameraComponent.up, glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.right * halfH)), cameraComponent.position);

				//TopFace
				cameraFrustum.top = FrustumFace(-glm::cross(cameraComponent.right, glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.up * halfV)), cameraComponent.position);

				//BottomFace
				cameraFrustum.bottom = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.up * halfV), cameraComponent.right), cameraComponent.position);

				//FarFace
				cameraFrustum.far = FrustumFace(-cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.farPlane);
			
				cameraFrustumBufferHandler[cameraIndex] = cameraFrustum;
			}
		}
	);
}

Entity CameraSystem::GetMainCameraEntity(std::shared_ptr<Registry> registry)
{
	auto cameraPool = registry->GetPool<CameraComponent>();

	if (!cameraPool)
		return NULL_ENTITY;;

	for (Entity entity : cameraPool->GetDenseIndices())
	{
		if (cameraPool->GetData(entity).isMain)
			return entity;
	}

	return NULL_ENTITY;
}
