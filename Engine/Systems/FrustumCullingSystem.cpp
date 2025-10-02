#include "FrustumCullingSystem.h"
#include "Engine/Components/DefaultColliderComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/ShapeComponent.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Physics/Collider/FrustumCollider.h"
#include "Engine/Physics/Simplex.h"
#include "Engine/Physics/Tester/TesterAABB.h"
#include "Engine/Physics/Tester/TesterGJK.h"
#include "Engine/Physics/Tester/TesterFrustum.h"
#include "Engine/Systems/CameraSystem.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

void FrustumCullingSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto cameraPool = registry->GetPool<CameraComponent>();
	if (!cameraPool)
		return;

	Entity cameraEntity = CameraSystem::GetMainCameraEntity(registry);
	auto& cameraComponent = cameraPool->GetData(cameraEntity);

	FrustumCollider frustumCollider{};

	float fovY = glm::radians(cameraComponent.fov);
	float aspectRatio = cameraComponent.width / cameraComponent.height;
	float halfV = cameraComponent.farPlane * tanf(fovY * 0.5f);
	float halfH = halfV * aspectRatio;

	//NearFace
	frustumCollider.faces[0] = FrustumFace(cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.nearPlane);
	
	//RightFace
	frustumCollider.faces[1] = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.right * halfH), cameraComponent.up), cameraComponent.position);

	//LeftFace
	frustumCollider.faces[2] = FrustumFace(-glm::cross(cameraComponent.up, glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.right * halfH)), cameraComponent.position);

	//TopFace
	frustumCollider.faces[3] = FrustumFace(-glm::cross(cameraComponent.right, glm::normalize(cameraComponent.direction * cameraComponent.farPlane + cameraComponent.up * halfV)), cameraComponent.position);

	//BottomFace
	frustumCollider.faces[4] = FrustumFace(-glm::cross(glm::normalize(cameraComponent.direction * cameraComponent.farPlane - cameraComponent.up * halfV), cameraComponent.right), cameraComponent.position);

	//FarFace
	frustumCollider.faces[5] = FrustumFace(-cameraComponent.direction, cameraComponent.position + cameraComponent.direction * cameraComponent.farPlane);

	//DefaultColliderCulling(registry, &frustumCollider);
	//PointLightCulling(registry, &frustumCollider);
	//SpotLightCulling(registry, &frustumCollider);
}

void FrustumCullingSystem::OnFinish(std::shared_ptr<Registry> registry)
{
}

void FrustumCullingSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
}

void FrustumCullingSystem::DefaultColliderCulling(std::shared_ptr<Registry> registry, FrustumCollider* cameraCollider)
{
	auto [defaultColliderPool, transformPool, shapePool, modelPool] = registry->GetPools<DefaultColliderComponent, TransformComponent, ShapeComponent, ModelComponent>();
	if (!defaultColliderPool || !transformPool)
		return;

	std::for_each(std::execution::par, defaultColliderPool->GetDenseIndices().begin(), defaultColliderPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			bool hasShape = shapePool && shapePool->HasComponent(entity) && shapePool->GetData(entity).shape;
			bool hasModel = modelPool && modelPool->HasComponent(entity) && modelPool->GetData(entity).model && modelPool->GetData(entity).model->state == LoadState::Ready;

			if (transformPool->HasComponent(entity) && (hasShape || hasModel))
			{
				auto& defaultColliderComponent = defaultColliderPool->GetData(entity);

				if (TesterFrustum::Test(cameraCollider, static_cast<ColliderAABB*>(&defaultColliderComponent)))
				{
					RenderComponent* renderComponent = hasShape ? static_cast<RenderComponent*>(&shapePool->GetData(entity)) : static_cast<RenderComponent*>(&modelPool->GetData(entity));
					renderComponent->toRender = true;
				}
			}
		}
	);
}

void FrustumCullingSystem::PointLightCulling(std::shared_ptr<Registry> registry, FrustumCollider* cameraCollider)
{
	auto [pointLightPool, transformPool] = registry->GetPools<PointLightComponent, TransformComponent>();
	if (!pointLightPool || !transformPool)
		return;

	std::for_each(std::execution::seq, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (transformPool->HasComponent(entity))
			{
				auto& pointLightComponent = pointLightPool->GetData(entity);

				SphereColliderGJK collider;
				collider.origin = pointLightComponent.position;
				collider.radius = pointLightComponent.radius;

				if (TesterFrustum::Test(cameraCollider, &collider))
				{
					pointLightComponent.toRender = true;
				}
			}
		}
	);
}

void FrustumCullingSystem::SpotLightCulling(std::shared_ptr<Registry> registry, FrustumCollider* cameraCollider)
{
	auto [spotLightPool, transformPool] = registry->GetPools<SpotLightComponent, TransformComponent>();
	if (!spotLightPool || !transformPool)
		return;

	std::for_each(std::execution::par, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (transformPool->HasComponent(entity))
			{
				auto& spotLightComponent = spotLightPool->GetData(entity);

				//Todo: Cone collider ->

				//if (TesterFrustum::Test(cameraCollider, &collider))
				if (true)
				{
					spotLightComponent.toRender = true;
				}
			}
		}
	);
}
