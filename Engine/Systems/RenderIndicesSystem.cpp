#include "RenderIndicesSystem.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/RenderIndicesComponent.h"
#include "Engine/Components/ShapeComponent.h"

void RenderIndicesSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [renderIndicesPool, transformPool, modelPool, shapePool] = registry->GetPools<RenderIndicesComponent, TransformComponent, ModelComponent, ShapeComponent>();

	if (!renderIndicesPool)
		return;

	std::for_each(std::execution::par, renderIndicesPool->GetDenseIndices().begin(), renderIndicesPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void 
		{
			bool changed = false;

			[[unlikely]]
			if (transformPool && transformPool->HasComponent(entity) && transformPool->IsBitSet<INDEX_CHANGED_BIT>(entity))
			{
				renderIndicesPool->GetData(entity).transformIndex = transformPool->GetDenseIndex(entity);
				changed = true;
			}

			[[unlikely]]
			if (modelPool && modelPool->HasComponent(entity) && modelPool->IsBitSet<INDEX_CHANGED_BIT>(entity))
			{
				renderIndicesPool->GetData(entity).objectIndex = modelPool->GetDenseIndex(entity);
				changed = true;
			}

			//Todo: Handle UINT32_MAX!

			//Todo: Pack model/shape into bitflag

			//Todo: Maybe problematic CHANGED_BIT and async load!

			[[unlikely]]
			if (modelPool && modelPool->HasComponent(entity) && modelPool->IsBitSet<CHANGED_BIT>(entity))
			{
				auto& modelComponent = modelPool->GetData(entity);

				uint32_t flags = 0;
				flags |= 1u << 0; // Bit 0 = Is Model
				flags |= (modelComponent.receiveShadow ? 1u : 0u) << 1; // Bit 1 = Receive Shadow
				flags |= (modelComponent.hasDirectxNormals ? 1u : 0u) << 2; // Bit 2 = Has DirectX Normals

				renderIndicesPool->GetData(entity).bitflag = flags;
				changed = true;
			}

			[[unlikely]]
			if (modelPool && modelPool->HasComponent(entity) && modelPool->GetData(entity).model && modelPool->GetData(entity).model->state == LoadState::Ready && modelPool->IsBitSet<CHANGED_BIT>(entity))
			{
				renderIndicesPool->GetData(entity).assetIndex = modelPool->GetData(entity).model->GetBufferArrayIndex();
				changed = true;
			}

			[[unlikely]]
			if (shapePool && shapePool->HasComponent(entity) && shapePool->IsBitSet<INDEX_CHANGED_BIT>(entity))
			{
				renderIndicesPool->GetData(entity).objectIndex = shapePool->GetDenseIndex(entity);
				changed = true;
			}

			[[unlikely]]
			if (shapePool && shapePool->HasComponent(entity) && shapePool->GetData(entity).shape && shapePool->IsBitSet<CHANGED_BIT>(entity))
			{
				auto& shapeComponent = shapePool->GetData(entity);

				uint32_t flags = 0;
				flags |= 0u << 0; // Bit 0 = Is Shape
				flags |= (shapeComponent.receiveShadow ? 1u : 0u) << 1; // Bit 1 = Receive Shadow

				renderIndicesPool->GetData(entity).assetIndex = shapeComponent.shape->GetBufferArrayIndex();
				renderIndicesPool->GetData(entity).bitflag = flags;
				changed = true;
			}

			[[unlikely]]
			if (changed)
			{
				renderIndicesPool->SetBit<CHANGED_BIT>(entity);
				renderIndicesPool->GetData(entity).version++;
			}
		}
	);
}

void RenderIndicesSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto renderIndicesPool = registry->GetPool<RenderIndicesComponent>();

	if (!renderIndicesPool)
		return;

	std::for_each(std::execution::par, renderIndicesPool->GetDenseIndices().begin(), renderIndicesPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void 
		{
			[[unlikely]]
			if (renderIndicesPool->IsBitSet<CHANGED_BIT>(entity))
				renderIndicesPool->GetBitset(entity).reset();
		}
	);
}

void RenderIndicesSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto renderIndicesPool = registry->GetPool<RenderIndicesComponent>();

	if (!renderIndicesPool)
		return;

	auto componentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("RenderIndicesData", frameIndex);
	auto bufferHandler = static_cast<RenderIndicesGPU*>(componentBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, renderIndicesPool->GetDenseIndices().begin(), renderIndicesPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& renderIndicesComponent = renderIndicesPool->GetData(entity);
			auto renderIndicesIndex = renderIndicesPool->GetDenseIndex(entity);

			if (componentBuffer->versions[renderIndicesIndex] != renderIndicesComponent.version)
			{
				componentBuffer->versions[renderIndicesIndex] = renderIndicesComponent.version;
				bufferHandler[renderIndicesIndex] = RenderIndicesGPU(renderIndicesComponent, entity);
			}
		}
	);
}
