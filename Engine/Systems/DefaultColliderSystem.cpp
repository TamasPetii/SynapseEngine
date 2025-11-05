#include "DefaultColliderSystem.h"

#include "Engine/Components/ShapeComponent.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Systems/CameraSystem.h"

void DefaultColliderSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [defaultColliderPool, transformPool, shapePool, modelPool, cameraPool] = registry->GetPools<DefaultColliderComponent, TransformComponent, ShapeComponent, ModelComponent, CameraComponent>();

	if (!defaultColliderPool || !transformPool || !cameraPool)
		return;

	Entity mainCameraEntity = CameraSystem::GetMainCameraEntity(registry);
	auto& cameraComponent = cameraPool->GetData(mainCameraEntity);
	bool cameraChanged = false && cameraPool && cameraPool->IsBitSet<CHANGED_BIT>(mainCameraEntity);

	std::for_each(std::execution::par, defaultColliderPool->GetDenseIndices().begin(), defaultColliderPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void
		{
			bool isShape = shapePool && shapePool->HasComponent(entity) && shapePool->GetData(entity).shape;
			bool isModel = modelPool && modelPool->HasComponent(entity) && modelPool->GetData(entity).model && modelPool->GetData(entity).model->state == LoadState::Ready;
			bool shapeChanged = isShape && shapePool->IsBitSet<CHANGED_BIT>(entity);
			bool modelChanged = isModel && modelPool->IsBitSet<CHANGED_BIT>(entity);
			
			if (transformPool->HasComponent(entity) && (isShape || isModel) && (defaultColliderPool->IsBitSet<UPDATE_BIT>(entity) || transformPool->IsBitSet<CHANGED_BIT>(entity) || shapeChanged || modelChanged || cameraChanged))
			{
				auto& defaultColliderComponent = defaultColliderPool->GetData(entity);
				auto& transformComponent = transformPool->GetData(entity);

				std::shared_ptr<BoundingVolume> boundingVolume = isShape ? std::static_pointer_cast<BoundingVolume>(shapePool->GetData(entity).shape) : std::static_pointer_cast<BoundingVolume>(modelPool->GetData(entity).model);

				glm::vec3 maxPosition{ std::numeric_limits<float>::lowest() };
				glm::vec3 minPosition{ std::numeric_limits<float>::max() };

				for (unsigned int i = 0; i < 8; ++i)
				{
					defaultColliderComponent.obbPositions[i] = glm::vec3(transformComponent.transform * glm::vec4(boundingVolume->obbPositions[i], 1));

					maxPosition = glm::max(maxPosition, defaultColliderComponent.obbPositions[i]);
					minPosition = glm::min(minPosition, defaultColliderComponent.obbPositions[i]);
				}

				defaultColliderComponent.aabbMin = minPosition;
				defaultColliderComponent.aabbMax = maxPosition;
				defaultColliderComponent.aabbOrigin = 0.5f * (minPosition + maxPosition);
				defaultColliderComponent.aabbExtents = 0.5f * (maxPosition - minPosition);

				defaultColliderComponent.origin = defaultColliderComponent.aabbOrigin;
				defaultColliderComponent.radius = glm::sqrt(glm::dot(defaultColliderComponent.aabbExtents, defaultColliderComponent.aabbExtents));

				defaultColliderPool->SetBit<CHANGED_BIT>(entity);
				defaultColliderComponent.version++;
			}
		}
	);
}

void DefaultColliderSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto [defaultColliderPool, transformPool] = registry->GetPools<DefaultColliderComponent, TransformComponent>();

	if (!defaultColliderPool || !transformPool)
		return;

	std::for_each(std::execution::par, defaultColliderPool->GetDenseIndices().begin(), defaultColliderPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if(defaultColliderPool->IsBitSet<CHANGED_BIT>(entity))
				defaultColliderPool->GetBitset(entity).reset();
		}
	);
}

void DefaultColliderSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [defaultColliderPool, transformPool, shapePool, modelPool] = registry->GetPools<DefaultColliderComponent, TransformComponent, ShapeComponent, ModelComponent>();

	if (!defaultColliderPool || !transformPool)
		return;

	auto componentBufferAabb = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderAabbData", frameIndex);
	auto bufferHandlerAabb = static_cast<glm::mat4*>(componentBufferAabb->buffer->GetHandler());

	auto componentBufferObb = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderObbData", frameIndex);
	auto bufferHandlerObb = static_cast<glm::mat4*>(componentBufferObb->buffer->GetHandler());

	auto componentBufferSphere = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderSphereData", frameIndex);
	auto bufferHandlerSphere = static_cast<glm::mat4*>(componentBufferSphere->buffer->GetHandler());

	auto defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex);
	auto defaultColliderHandler = static_cast<DefaultColliderGPU*>(defaultColliderBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, defaultColliderPool->GetDenseIndices().begin(), defaultColliderPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (transformPool->HasComponent(entity))
			{
				auto& defaultColliderComponent = defaultColliderPool->GetData(entity);
				auto& transformComponent = transformPool->GetData(entity);
				auto defaultColliderIndex = defaultColliderPool->GetDenseIndex(entity);

				if (GlobalConfig::WireframeConfig::showColliderAABB && componentBufferAabb->versions[defaultColliderIndex] != defaultColliderComponent.version)
				{
					componentBufferAabb->versions[defaultColliderIndex] = defaultColliderComponent.version;
					bufferHandlerAabb[defaultColliderIndex] = glm::translate(defaultColliderComponent.aabbOrigin) * glm::scale(defaultColliderComponent.aabbExtents);
				}

				if (GlobalConfig::WireframeConfig::showColliderOBB && componentBufferObb->versions[defaultColliderIndex] != defaultColliderComponent.version)
				{
					bool isShape = shapePool && shapePool->HasComponent(entity) && shapePool->GetData(entity).shape != nullptr;
					std::shared_ptr<BoundingVolume> boundingVolume = isShape ? std::static_pointer_cast<BoundingVolume>(shapePool->GetData(entity).shape) : std::static_pointer_cast<BoundingVolume>(modelPool->GetData(entity).model);

					componentBufferObb->versions[defaultColliderIndex] = defaultColliderComponent.version;
					bufferHandlerObb[defaultColliderIndex] = transformComponent.transform * glm::translate(boundingVolume->aabbOrigin) * glm::scale(boundingVolume->aabbExtents);
				}

				if (GlobalConfig::WireframeConfig::showColliderSphere && componentBufferSphere->versions[defaultColliderIndex] != defaultColliderComponent.version)
				{
					componentBufferSphere->versions[defaultColliderIndex] = defaultColliderComponent.version;
					bufferHandlerSphere[defaultColliderIndex] = glm::translate(defaultColliderComponent.origin) * glm::scale(glm::vec3(defaultColliderComponent.radius));
				}
				
				//Todo: Default Collider Component should always be on Model/Shape and should get destroyed when no component!
				if (defaultColliderBuffer->versions[defaultColliderIndex] != defaultColliderComponent.version)
				{
					defaultColliderBuffer->versions[defaultColliderIndex] = defaultColliderComponent.version;

					bool isShape = shapePool && shapePool->HasComponent(entity) && shapePool->GetData(entity).shape;
					bool isModel = modelPool && modelPool->HasComponent(entity) && modelPool->GetData(entity).model && modelPool->GetData(entity).model->state == LoadState::Ready;

					uint32_t objectIndex = isShape ? shapePool->GetDenseIndex(entity) : isModel ? modelPool->GetDenseIndex(entity) : UINT32_MAX;

					defaultColliderHandler[defaultColliderIndex] = DefaultColliderGPU(defaultColliderComponent, objectIndex, isModel);
				}
			}
		}
	);
}