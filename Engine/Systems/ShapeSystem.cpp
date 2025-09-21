#include "ShapeSystem.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Components/ShapeComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/MaterialComponent.h"
#include "Engine/Components/RenderIndicesComponent.h"

void ShapeSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [shapePool, transformPool, materialPool] = registry->GetPools<ShapeComponent, TransformComponent, MaterialComponent>();

	if (!shapePool)
		return;

	std::for_each(std::execution::par_unseq, shapePool->GetDenseIndices().begin(), shapePool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if (shapePool->IsBitSet<UPDATE_BIT>(entity) ||
				(transformPool && transformPool->HasComponent(entity) && transformPool->IsBitSet<INDEX_CHANGED_BIT>(entity)) ||
				(materialPool && materialPool->HasComponent(entity) && materialPool->IsBitSet<INDEX_CHANGED_BIT>(entity))
			){
				shapePool->SetBit<CHANGED_BIT>(entity);
				shapePool->GetData(entity).version++;
			}
		}
	);
}

void ShapeSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto shapePool = registry->GetPool<ShapeComponent>();

	if (!shapePool)
		return;

	std::for_each(std::execution::par_unseq, shapePool->GetDenseIndices().begin(), shapePool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			shapePool->GetData(entity).toRender = false;

			[[unlikely]]
			if(shapePool->IsBitSet<CHANGED_BIT>(entity))
				shapePool->GetBitset(entity).reset();
		}
	);
}

void ShapeSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [shapePool, transformPool, materialPool] = registry->GetPools<ShapeComponent, TransformComponent, MaterialComponent>();

	if (!shapePool)
		return;

	auto renderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex);
	auto renderIndicesBufferHandler = static_cast<ShapeRenderIndicesGPU*>(renderIndicesBuffer->buffer->GetHandler());

	std::for_each(std::execution::par_unseq, shapePool->GetDenseIndices().begin(), shapePool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& shapeComponent = shapePool->GetData(entity);
			auto shapeIndex = shapePool->GetDenseIndex(entity);

			bool hasShape = shapeComponent.shape != nullptr;
		
			if (hasShape)
			{
				uint32_t flags = 0;
				flags |= (shapeComponent.receiveShadow ? 1u : 0u) << 0; // Bit 0

				auto renderIndices = ShapeRenderIndicesGPU{
					.entityIndex = entity,
					.transformIndex = transformPool && transformPool->HasComponent(entity) ? transformPool->GetDenseIndex(entity) : UINT32_MAX,
					.shapeIndex = shapeComponent.shape->GetBufferArrayIndex(),
					.materialIndex = materialPool && materialPool->HasComponent(entity) ? materialPool->GetDenseIndex(entity) : UINT32_MAX,
					.bitflag = flags
				};

				renderIndicesBufferHandler[shapeIndex] = renderIndices;
			}


			//MAYBE EACH SYSTEM UPDATES ITS INDEX????

			/*
			[[unlikely]]
			if (renderIndicesBuffer->versions[modelIndex] != modelComponent.versionID)
			{
				renderIndicesBuffer->versions[modelIndex] = modelComponent.versionID;

				uint32_t flags = 0;
				flags |= (modelComponent.receiveShadow ? 1u : 0u) << 0;       // Bit 0
				flags |= (modelComponent.hasDirectxNormals ? 1u : 0u) << 1;    // Bit 1

				auto renderIndices = RenderIndicesGPU{
					.entityIndex = entity,
					.transformIndex = transformPool && transformPool->HasComponent(entity) ? transformPool->GetDenseIndex(entity) : UINT32_MAX,
					.modelIndex = modelIndex,
					.animationIndex = hasAnimation ? animationPool->GetDenseIndex(entity) : UINT32_MAX,
					.animationComponentIndex = hasAnimation ? animationPool->GetData(entity).animation->GetAddressArrayIndex() : UINT32_MAX,
					.bitflag = flags,
				};

				renderIndicesBufferHandler[modelIndex] = renderIndices;
			}
			*/
		}
	);

	/*
	auto componentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex);
	auto bufferHandler = static_cast<RenderIndicesGPU*>(componentBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, shapePool->GetDenseIndices().begin(), shapePool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& shapeComponent = shapePool->GetData(entity);
			auto shapeIndex = shapePool->GetDenseIndex(entity);

			[[unlikely]]
			if (componentBuffer->versions[shapeIndex] != shapeComponent.versionID)
			{
				componentBuffer->versions[shapeIndex] = shapeComponent.versionID;

				uint32_t flags = 0;
				flags |= (shapeComponent.receiveShadow ? 1u : 0u) << 0;       // Bit 0

				bufferHandler[shapeIndex] = RenderIndicesGPU{
					.entityIndex = entity,
					.transformIndex = transformPool && transformPool->HasComponent(entity) ? transformPool->GetDenseIndex(entity) : NULL_ENTITY,
					.materialIndex = materialPool && materialPool->HasComponent(entity) ? materialPool->GetDenseIndex(entity) : NULL_ENTITY,
					.receiveShadow = flags
				};
			}
		}
	);
	*/
}
