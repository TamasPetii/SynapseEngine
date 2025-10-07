#include "TransformSystem.h"
#include "Engine/Components/TransformComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/matrix_decompose.hpp>

void TransformSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [transformPool, relationShipPool] = registry->GetPools<TransformComponent, Relationship>();
	
	if (!transformPool)
		return;

	if (!transformPool->BitsetFlagged::IsBitSet<UPDATE_BIT>())
		return;

	//Todo: FrameIndex BitsetFlagged -> GPU cannot skip!!!

	// Iterating over all the entities, checking level depth of parent-child relation
	// Optimization: Get max depth in registry, probably less than MAX_RELATIONSHIP_DEPTH
	for (uint32_t depth = 0; depth < MAX_RELATIONSHIP_DEPTH; ++depth)
	{
		std::for_each(std::execution::par, transformPool->GetDenseIndices().begin(), transformPool->GetDenseIndices().end(),
			[&](const Entity& entity) -> void {
				auto& relationShipComponent = relationShipPool->GetData(entity);
				if (relationShipComponent.level == depth && (transformPool->IsBitSet<UPDATE_BIT>(entity) || (relationShipComponent.parent != NULL_ENTITY && transformPool->IsBitSet<CHANGED_BIT>(relationShipComponent.parent))))
				{
					auto& transformComponent = transformPool->GetData(entity);

					transformComponent.transform = glm::mat4(1.0f);
					transformComponent.transform = glm::translate(transformComponent.transform, transformComponent.translation);
					transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.z), glm::vec3(0, 0, 1));
					transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.y), glm::vec3(0, 1, 0));
					transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.x), glm::vec3(1, 0, 0));
					transformComponent.transform = glm::scale(transformComponent.transform, transformComponent.scale);

					if (relationShipComponent.parent != NULL_ENTITY && transformPool->HasComponent(relationShipComponent.parent))
						transformComponent.transform = transformPool->GetData(relationShipComponent.parent).transform * transformComponent.transform;

					transformComponent.transformIT = glm::transpose(glm::inverse(transformComponent.transform));

					transformPool->SetBit<CHANGED_BIT>(entity);
					transformComponent.version++;
				}
			}
		);
	}

	/*
	// Entities with given level stored in std::set -> Much slower in release mode
	for (uint32_t depth = 0; depth < MAX_RELATIONSHIP_DEPTH; ++depth)
	{
		auto& levelEntities = registry->GetLevels()[depth];

		if (levelEntities.empty())
			break;

		std::for_each(std::execution::par, levelEntities.begin(), levelEntities.end(),
			[&](const Entity& entity) -> void {
				auto& relationShipComponent = relationShipPool->GetData(entity);
				if (transformPool->HasComponent(entity) && (transformPool->IsBitSet<UPDATE_BIT>(entity) || (relationShipComponent.parent != NULL_ENTITY && transformPool->IsBitSet<CHANGED_BIT>(relationShipComponent.parent))))
				{
					auto& transformComponent = transformPool->GetData(entity);

					transformComponent.transform = glm::mat4(1.0f);
					transformComponent.transform = glm::translate(transformComponent.transform, transformComponent.translation);
					transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.z), glm::vec3(0, 0, 1));
					transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.y), glm::vec3(0, 1, 0));
					transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.x), glm::vec3(1, 0, 0));
					transformComponent.transform = glm::scale(transformComponent.transform, transformComponent.scale);

					if (relationShipComponent.parent != NULL_ENTITY && transformPool->HasComponent(relationShipComponent.parent))
						transformComponent.transform = transformPool->GetData(relationShipComponent.parent).transform * transformComponent.transform;

					transformComponent.transformIT = glm::inverse(glm::transpose(transformComponent.transform));

					transformPool->SetBit<CHANGED_BIT>(entity);
					transformComponent.versionID++;
				}
			}
		);
	}
	*/
}

void TransformSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto transformPool = registry->GetPool<TransformComponent>();

	if (!transformPool)
		return;

	if (!transformPool->BitsetFlagged::IsBitSet<CHANGED_BIT>())
		return;

	std::for_each(std::execution::par, transformPool->GetDenseIndices().begin(), transformPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if(transformPool->IsBitSet<CHANGED_BIT>(entity))
				transformPool->GetBitset(entity).reset();
		}
	);

	transformPool->BitsetFlagged::GetBitset().reset();
}

void TransformSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto transformPool = registry->GetPool<TransformComponent>();

	if (!transformPool)
		return;

	auto componentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex);
	auto bufferHandler = static_cast<TransformComponentGPU*>(componentBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, transformPool->GetDenseIndices().begin(), transformPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& transformComponent = transformPool->GetData(entity);
			auto transformIndex = transformPool->GetDenseIndex(entity);

			if (componentBuffer->versions[transformIndex] != transformComponent.version)
			{
				componentBuffer->versions[transformIndex] = transformComponent.version;
				bufferHandler[transformIndex] = TransformComponentGPU(transformComponent);
			}
		}
	);
}
