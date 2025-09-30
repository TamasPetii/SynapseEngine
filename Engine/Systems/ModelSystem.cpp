#include "ModelSystem.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/AnimationComponent.h"
#include "Engine/Components/RenderIndicesComponent.h"

void ModelSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [modelPool, transformPool] = registry->GetPools<ModelComponent, TransformComponent>();

	if (!modelPool)
		return;

	std::for_each(std::execution::par, modelPool->GetDenseIndices().begin(), modelPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if (modelPool->IsBitSet<UPDATE_BIT>(entity) || (transformPool && transformPool->HasComponent(entity) && transformPool->IsBitSet<INDEX_CHANGED_BIT>(entity)))
			{
				modelPool->SetBit<CHANGED_BIT>(entity);
				modelPool->GetData(entity).version++;
			}
		}
	);
}

void ModelSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto modelPool = registry->GetPool<ModelComponent>();

	if (!modelPool)
		return;

	std::for_each(std::execution::par, modelPool->GetDenseIndices().begin(), modelPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			modelPool->GetData(entity).toRender = false;

			[[unlikely]]
			if(modelPool->IsBitSet<CHANGED_BIT>(entity))
				modelPool->GetBitset(entity).reset();
		}
	);
}

void ModelSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [modelPool, transformPool, animationPool] = registry->GetPools<ModelComponent, TransformComponent, AnimationComponent>();

	if (!modelPool)
		return;

	auto renderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex);
	auto renderIndicesBufferHandler = static_cast<ModelRenderIndicesGPU*>(renderIndicesBuffer->buffer->GetHandler());

	std::for_each(std::execution::seq, modelPool->GetDenseIndices().begin(), modelPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& modelComponent = modelPool->GetData(entity);
			auto modelIndex = modelPool->GetDenseIndex(entity);

			bool hasModel = modelComponent.model && modelComponent.model->state == LoadState::Ready;
			bool hasAnimation = animationPool && animationPool->HasComponent(entity) && animationPool->GetData(entity).animation && animationPool->GetData(entity).animation->state == LoadState::Ready;

			if (hasModel)
			{
				uint32_t flags = 0;
				flags |= (modelComponent.receiveShadow ? 1u : 0u) << 0;        // Bit 0
				flags |= (modelComponent.hasDirectxNormals ? 1u : 0u) << 1;    // Bit 1

				auto renderIndices = ModelRenderIndicesGPU{
					.entityIndex = entity,
					.transformIndex = transformPool && transformPool->HasComponent(entity) ? transformPool->GetDenseIndex(entity) : UINT32_MAX,
					.modelIndex = hasModel ? modelComponent.model->GetBufferArrayIndex() : UINT32_MAX,
					.animationIndex = hasAnimation ? animationPool->GetData(entity).animation->GetBufferArrayIndex() : UINT32_MAX,
					.animationTransformIndex = hasAnimation ? animationPool->GetDenseIndex(entity) : UINT32_MAX,
					.bitflag = flags
				};

				renderIndicesBufferHandler[modelIndex] = renderIndices;

				//std::cout << std::format("Render Indices ModelComponent {} | {} {} {} {} {}\n", modelIndex, renderIndices.entityIndex, renderIndices.transformIndex, renderIndices.modelIndex, renderIndices.animationIndex, renderIndices.animationTransformIndex);
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
}
