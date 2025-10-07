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
	//auto futurePointLight = std::async(std::launch::async, &InstanceSystem::UpdatePointLightInstances, this, registry);
	//auto futureSpotLight = std::async(std::launch::async, &InstanceSystem::UpdateSpotLightInstances, this, registry);

	futureShape.get();
	futureModel.get();
	//futurePointLight.get();
	//futureSpotLight.get();
}

void InstanceSystem::OnFinish(std::shared_ptr<Registry> registry)
{
}

void InstanceSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto futureShapeGpu = std::async(std::launch::async, &InstanceSystem::UpdateShapeInstancesGpuIndirectDraw, this, resourceManager, frameIndex);
	auto futureModelGpu = std::async(std::launch::async, &InstanceSystem::UpdateModelInstancesGpuIndirectDraw, this, resourceManager, frameIndex);
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

void InstanceSystem::UpdateShapeInstancesGpuNew(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto geometryManager = resourceManager->GetGeometryManager();
	auto shapeIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(geometryManager->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());

	std::for_each(std::execution::par, geometryManager->GetShapes().begin(), geometryManager->GetShapes().end(),
		[&](const auto& data) -> void
		{
			if (data.second == nullptr)
				return;

			auto shape = data.second->object;

			if (shape)
			{
				uint32_t instanceCount = shape->GetInstanceIndices().size();

				//Update insatnce index buffer with visible model indices
				VkDeviceSize bufferSize = sizeof(uint32_t) * instanceCount;
				auto shapeInstanceIndexBufferHandler = geometryManager->GetInstanceIndexBuffer(frameIndex, shape->GetBufferArrayIndex())->buffer->GetHandler();
				memcpy(shapeInstanceIndexBufferHandler, shape->GetInstanceIndices().data(), (size_t)bufferSize);

				shape->ClearInstanceIndices();
			}
		}
	);
}

void InstanceSystem::UpdateShapeInstancesGpuIndirectDraw(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto geometryManager = resourceManager->GetGeometryManager();	
	auto shapeIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(geometryManager->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	
	std::for_each(std::execution::seq, geometryManager->GetShapes().begin(), geometryManager->GetShapes().end(),
		[&](const auto& data) -> void
		{
			if (data.second == nullptr)
				return;

			auto shape = data.second->object;

			//Todo: On shape deletion instanceCount has to be 0!!
			if (shape)
			{
				uint32_t instanceCount = shape->GetInstanceIndices().size();

				//Update insatnce index buffer with visible model indices
				VkDeviceSize bufferSize = sizeof(uint32_t) * instanceCount;
				auto shapeInstanceIndexBufferHandler = geometryManager->GetInstanceIndexBuffer(frameIndex, shape->GetBufferArrayIndex())->buffer->GetHandler();
				memcpy(shapeInstanceIndexBufferHandler, shape->GetInstanceIndices().data(), (size_t)bufferSize);

				//Update indirect draw buffer with instance count!
				shapeIndirectDrawBufferHandler[shape->GetBufferArrayIndex()].instanceCount = instanceCount;

				shape->ClearInstanceIndices();
			}
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

void InstanceSystem::UpdateModelInstancesGpuNew(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto modelManager = resourceManager->GetModelManager();
	std::for_each(std::execution::seq, modelManager->GetModels().begin(), modelManager->GetModels().end(),
		[&](const auto& data) -> void
		{
			if (data.second == nullptr)
				return;

			auto model = data.second->object;

			if (model && model->state == LoadState::Ready)
			{
				uint32_t instanceCount = model->GetInstanceIndices().size();

				//Update insatnce index buffer with visible model indices
				VkDeviceSize bufferSize = sizeof(uint32_t) * instanceCount;
				auto modelInstanceIndexBufferHandler = modelManager->GetInstanceIndexBuffer(frameIndex, model->GetBufferArrayIndex())->buffer->GetHandler();
				memcpy(modelInstanceIndexBufferHandler, model->GetInstanceIndices().data(), (size_t)bufferSize);

				model->ClearInstanceIndices();
			}
		}
	);
}

void InstanceSystem::UpdateModelInstancesGpuIndirectDraw(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto modelManager = resourceManager->GetModelManager();
	auto modelIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(modelManager->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());

	std::for_each(std::execution::seq, modelManager->GetModels().begin(), modelManager->GetModels().end(),
		[&](const auto& data) -> void
		{
			auto model = data.second->object;

			//Todo: On model deletion instanceCount has to be 0!!
			if (model && model->state == LoadState::Ready)
			{
				uint32_t instanceCount = model->GetInstanceIndices().size();

				//Update insatnce index buffer with visible model indices
				VkDeviceSize bufferSize = sizeof(uint32_t) * instanceCount;
				auto modelInstanceIndexBufferHandler = modelManager->GetInstanceIndexBuffer(frameIndex, model->GetBufferArrayIndex())->buffer->GetHandler();
				memcpy(modelInstanceIndexBufferHandler, model->GetInstanceIndices().data(), (size_t)bufferSize);

				//Update indirect draw buffer with instance count!
				modelIndirectDrawBufferHandler[model->GetBufferArrayIndex()].instanceCount = instanceCount;

				model->ClearInstanceIndices();
			}	
		}
	);
}

void InstanceSystem::UpdatePointLightInstancesGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();
	if (!pointLightPool)
		return;

	auto pointLightManager = resourceManager->GetPointLightBufferManager();
	auto pointLightIndirectDrawBuffer = static_cast<VkDrawIndirectCommand*>(pointLightManager->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	auto pointLightInstanceIndexBufferHandler = static_cast<uint32_t*>(pointLightManager->GetInstanceIndexBuffer(frameIndex)->buffer->GetHandler());

	std::atomic<uint32_t> instanceCount = 0;

	std::for_each(std::execution::seq, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](Entity entity) -> void {
			auto& pointLightComponent = pointLightPool->GetData(entity);

			if (pointLightComponent.toRender)
				pointLightInstanceIndexBufferHandler[instanceCount++] = pointLightPool->GetDenseIndex(entity);
		}
	);

	pointLightIndirectDrawBuffer[0].instanceCount = instanceCount;
}

void InstanceSystem::UpdateSpotLightInstancesGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();
	if (!spotLightPool)
		return;

	auto spotLightManager = resourceManager->GetSpotLightBufferManager();
	auto spotLightIndirectDrawBuffer = static_cast<VkDrawIndirectCommand*>(spotLightManager->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	auto spotLightInstanceIndexBufferHandler = static_cast<uint32_t*>(spotLightManager->GetInstanceIndexBuffer(frameIndex)->buffer->GetHandler());

	std::atomic<uint32_t> instanceCount = 0;

	std::for_each(std::execution::seq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](Entity entity) -> void {
			auto& spotLightComponent = spotLightPool->GetData(entity);

			if (spotLightComponent.toRender)
				spotLightInstanceIndexBufferHandler[instanceCount++] = spotLightPool->GetDenseIndex(entity);
		}
	);

	spotLightIndirectDrawBuffer[0].instanceCount = instanceCount;
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

	return !(alpha < spotLightComponent.angles.w || delta > spotLightComponent.range);
}