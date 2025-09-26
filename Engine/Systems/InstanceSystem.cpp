#include "InstanceSystem.h"
#include "CameraSystem.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/ShapeComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

#include <future>
#include <thread>
#include <ranges>
#include <algorithm>

#include "Engine/Timer/Timer.h"

void InstanceSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto futureShape = std::async(std::launch::async, &InstanceSystem::UpdateShapeInstances, this, registry, resourceManager);
	auto futureModel = std::async(std::launch::async, &InstanceSystem::UpdateModelInstances, this, registry, resourceManager);
	auto futurePointLight = std::async(std::launch::async, &InstanceSystem::UpdatePointLightInstances, this, registry);
	auto futureSpotLight = std::async(std::launch::async, &InstanceSystem::UpdateSpotLightInstances, this, registry);

	futureShape.get();
	futureModel.get();
	futurePointLight.get();
	futureSpotLight.get();
}

void InstanceSystem::OnFinish(std::shared_ptr<Registry> registry)
{
}

void InstanceSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto futureShapeGpu = std::async(std::launch::async, &InstanceSystem::UpdateShapeInstancesGpu, this, resourceManager, frameIndex);
	auto futureModelGpu = std::async(std::launch::async, &InstanceSystem::UpdateModelInstancesGpu, this, resourceManager, frameIndex);
	auto futurePointLightGpu = std::async(std::launch::async, &InstanceSystem::UpdatePointLightInstancesGpu, this, registry, resourceManager, frameIndex);
	auto futureSpotLightGpu = std::async(std::launch::async, &InstanceSystem::UpdateSpotLightInstancesGpu, this, registry, resourceManager, frameIndex);

	futureShapeGpu.get();
	futureModelGpu.get();
	futurePointLightGpu.get();
	futureSpotLightGpu.get();
}

void InstanceSystem::UpdateShapeInstances(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager)
{
	auto geometryManager = resourceManager->GetGeometryManager();
	for (auto& [name, versionedObject] : geometryManager->GetShapes())
	{
		if (versionedObject == nullptr)
			continue;

		auto shape = versionedObject->object;

		shape->ResetInstanceCount();
		shape->ReserveInstances(shape.use_count());
	}

	auto shapePool = registry->GetPool<ShapeComponent>();
	if (!shapePool)
		return;

	std::for_each(std::execution::seq, shapePool->GetDenseIndices().begin(), shapePool->GetDenseIndices().end(),
		[&](Entity entity) -> void {
			auto& shapeComponent = shapePool->GetData(entity);

			if (shapeComponent.toRender && shapeComponent.shape)
				shapeComponent.shape->AddIndex(shapePool->GetDenseIndex(entity));
		}
	);

	for (auto& [name, versionedObject] : geometryManager->GetShapes())
	{
		if (versionedObject == nullptr)
			continue;

		auto shape = versionedObject->object;

		shape->ShrinkInstances();
	}
}

void InstanceSystem::UpdateShapeInstancesGpu(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto geometryManager = resourceManager->GetGeometryManager();
	std::for_each(std::execution::par, geometryManager->GetShapes().begin(), geometryManager->GetShapes().end(),
		[&](const auto& data) -> void 
		{
			if (data.second == nullptr)
				return;

			auto shape = data.second->object;

			if (shape)
				shape->UploadInstanceDataToGPU(shape.use_count(), frameIndex);
		}
	);
}

void InstanceSystem::UpdateModelInstances(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager)
{
	auto modelManager = resourceManager->GetModelManager();
	for (auto& [name, versionedObject] : modelManager->GetModels())
	{
		if (versionedObject == nullptr)
			continue;

		auto model = versionedObject->object;

		if (model && model->state == LoadState::Ready)
		{
			model->ResetInstanceCount();
			model->ReserveInstances(model.use_count());
		}
	}

	auto modelPool = registry->GetPool<ModelComponent>();
	if (!modelPool)
		return;

	std::for_each(std::execution::seq, modelPool->GetDenseIndices().begin(), modelPool->GetDenseIndices().end(),
		[&](Entity entity) -> void 	{
			auto& modelComponent = modelPool->GetData(entity);

			if (modelComponent.toRender && modelComponent.model && modelComponent.model->state == LoadState::Ready)
				modelComponent.model->AddIndex(modelPool->GetDenseIndex(entity));
		}
	);

	for (auto& [name, versionedObject] : modelManager->GetModels())
	{
		if (versionedObject == nullptr)
			continue;

		auto model = versionedObject->object;

		if (model && model->state == LoadState::Ready)
			model->ShrinkInstances();
	}
}

void InstanceSystem::UpdateModelInstancesGpu(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto modelManager = resourceManager->GetModelManager();
	std::for_each(std::execution::par, modelManager->GetModels().begin(), modelManager->GetModels().end(),
		[&](const auto& data) -> void 
		{
			if (data.second == nullptr)
				return;

			auto model = data.second->object;

			if(model && model->state == LoadState::Ready)
				model->UploadInstanceDataToGPU(model.use_count(), frameIndex);
		}
	);
}

void InstanceSystem::UpdatePointLightInstances(std::shared_ptr<Registry> registry)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();
	if (!pointLightPool)
		return;

	PointLightComponent::instanceCount = 0;
	PointLightComponent::instanceIndices.clear();
	PointLightComponent::instanceIndices.reserve(pointLightPool->GetDenseSize());

	std::for_each(std::execution::seq, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](Entity entity) -> void {
			auto& pointLightComponent = pointLightPool->GetData(entity);

			if (pointLightComponent.toRender)
			{
				PointLightComponent::instanceIndices.push_back(pointLightPool->GetDenseIndex(entity));
				PointLightComponent::instanceCount++;
			}
		}
	);

	PointLightComponent::instanceIndices.resize(PointLightComponent::instanceCount);
}

void InstanceSystem::UpdatePointLightInstancesGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();
	if (!pointLightPool || PointLightComponent::instanceCount == 0)
		return;

	{
		VkDeviceSize bufferSize = sizeof(uint32_t) * PointLightComponent::instanceCount;
		auto pointLightInstanceIndicesBufferHandler = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightInstanceIndices", frameIndex)->buffer->GetHandler();
		memcpy(pointLightInstanceIndicesBufferHandler, PointLightComponent::instanceIndices.data(), (size_t)bufferSize);
	}

	{
		VkDeviceSize bufferSize = sizeof(uint32_t) * PointLightComponent::instanceCount;
		auto pointLightOcclusionIndicesBufferHandler = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightOcclusionIndices", frameIndex)->buffer->GetHandler();
		memset(pointLightOcclusionIndicesBufferHandler, 0, (size_t)bufferSize);
	}
}

void InstanceSystem::UpdatePointLightInstancesWithOcclusion(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [pointLightPool, cameraPool] = registry->GetPools<PointLightComponent, CameraComponent>();

	if (!pointLightPool || !cameraPool || PointLightComponent::instanceCount == 0)
		return;

	auto mainCameraEntity = CameraSystem::GetMainCameraEntity(registry);
	auto& cameraComponent = cameraPool->GetData(mainCameraEntity);

	auto pointLightOcclusionIndicesBufferHandler = static_cast<uint32_t*>(resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightOcclusionIndices", frameIndex)->buffer->GetHandler());

	uint32_t currentIndex = 0;
	for (uint32_t i = 0; i < PointLightComponent::instanceCount; ++i)
	{
		auto& pointLightComponent = pointLightPool->GetDenseData()[PointLightComponent::instanceIndices[i]];

		//Camera in inside the light volume it won't be rendered which means no fragments created because of Back Face Culling.
		//In this case we still need to add the instance index to the final instance indices list.
		if (pointLightOcclusionIndicesBufferHandler[i] == 1 || IsCameraInsidePointLightCubeVolume(pointLightComponent, cameraComponent))
			PointLightComponent::instanceIndices[currentIndex++] = PointLightComponent::instanceIndices[i];
	}

	PointLightComponent::instanceCount = currentIndex;
	PointLightComponent::instanceIndices.resize(PointLightComponent::instanceCount);

	VkDeviceSize bufferSize = sizeof(uint32_t) * PointLightComponent::instanceCount;
	auto pointLightInstanceIndicesBufferHandler = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightInstanceIndices", frameIndex)->buffer->GetHandler();
	memcpy(pointLightInstanceIndicesBufferHandler, PointLightComponent::instanceIndices.data(), (size_t)bufferSize);
}

bool InstanceSystem::IsCameraInsidePointLightSphereVolume(const PointLightComponent& pointLightComponent, const CameraComponent& cameraComponent)
{
	return glm::distance(pointLightComponent.position, cameraComponent.position) - 0.01f < pointLightComponent.radius;
}

bool InstanceSystem::IsCameraInsidePointLightCubeVolume(const PointLightComponent& pointLightComponent, const CameraComponent& cameraComponent)
{
	glm::vec3 difference = glm::abs(pointLightComponent.position - cameraComponent.position) - 0.01f;
	return difference.x < pointLightComponent.radius && difference.y < pointLightComponent.radius && difference.z < pointLightComponent.radius;
}

bool InstanceSystem::IsCameraInsideSpotLightConeVolume(const SpotLightComponent& spotLightComponent, const CameraComponent& cameraComponent)
{
	glm::vec3 fromSpotToCamera = cameraComponent.position - spotLightComponent.position;
	glm::vec3 fromSpotToCameraNorm = glm::normalize(fromSpotToCamera);
	glm::vec3 spotDirNorm = glm::normalize(spotLightComponent.direction);
	float alpha = glm::dot(spotDirNorm, fromSpotToCameraNorm); // Angle: tells if we are inside of the cone
	float delta = glm::dot(spotDirNorm, fromSpotToCamera); //The projected length to check if its inside the far plane

	return !(alpha < spotLightComponent.angles.w || delta > spotLightComponent.length);
}

void InstanceSystem::UpdateSpotLightInstances(std::shared_ptr<Registry> registry)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();
	if (!spotLightPool)
		return;

	SpotLightComponent::instanceCount = 0;
	SpotLightComponent::instanceIndices.clear();
	SpotLightComponent::instanceIndices.reserve(spotLightPool->GetDenseSize());

	std::for_each(std::execution::seq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](Entity entity) -> void {
			auto& spotLightComponent = spotLightPool->GetData(entity);

			if (spotLightComponent.toRender)
			{
				SpotLightComponent::instanceIndices.push_back(spotLightPool->GetDenseIndex(entity));
				SpotLightComponent::instanceCount++;
			}
		}
	);

	SpotLightComponent::instanceIndices.resize(SpotLightComponent::instanceCount);
}

void InstanceSystem::UpdateSpotLightInstancesGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();
	if (!spotLightPool || SpotLightComponent::instanceCount == 0)
		return;

	{
		VkDeviceSize bufferSize = sizeof(uint32_t) * SpotLightComponent::instanceCount;
		auto spotLightInstanceIndicesBufferHandler = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightInstanceIndices", frameIndex)->buffer->GetHandler();
		memcpy(spotLightInstanceIndicesBufferHandler, SpotLightComponent::instanceIndices.data(), (size_t)bufferSize);
	}

	{
		VkDeviceSize bufferSize = sizeof(uint32_t) * SpotLightComponent::instanceCount;
		auto spotLightOcclusionIndicesBufferHandler = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightOcclusionIndices", frameIndex)->buffer->GetHandler();
		memset(spotLightOcclusionIndicesBufferHandler, 0, (size_t)bufferSize);
	}
}

void InstanceSystem::UpdateSpotLightInstancesWithOcclusion(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [spotLightPool, cameraPool] = registry->GetPools<SpotLightComponent, CameraComponent>();

	if (!spotLightPool || !cameraPool || SpotLightComponent::instanceCount == 0)
		return;

	auto mainCameraEntity = CameraSystem::GetMainCameraEntity(registry);
	auto& cameraComponent = cameraPool->GetData(mainCameraEntity);

	auto spotLightOcclusionIndicesBufferHandler = static_cast<uint32_t*>(resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightOcclusionIndices", frameIndex)->buffer->GetHandler());

	uint32_t currentIndex = 0;
	for (uint32_t i = 0; i < SpotLightComponent::instanceCount; ++i)
	{
		auto& spotLightComponent = spotLightPool->GetDenseData()[SpotLightComponent::instanceIndices[i]];

		//Camera in inside the light volume it won't be rendered which means no fragments created because of Back Face Culling.
		//In this case we still need to add the instance index to the final instance indices list.
		if (spotLightOcclusionIndicesBufferHandler[i] == 1 || IsCameraInsideSpotLightConeVolume(spotLightComponent, cameraComponent))
			SpotLightComponent::instanceIndices[currentIndex++] = SpotLightComponent::instanceIndices[i];
	}

	SpotLightComponent::instanceCount = currentIndex;
	SpotLightComponent::instanceIndices.resize(SpotLightComponent::instanceCount);

	VkDeviceSize bufferSize = sizeof(uint32_t) * SpotLightComponent::instanceCount;
	auto spotLightInstanceIndicesBufferHandler = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightInstanceIndices", frameIndex)->buffer->GetHandler();
	memcpy(spotLightInstanceIndicesBufferHandler, SpotLightComponent::instanceIndices.data(), (size_t)bufferSize);
}
