#include "DirectionLightSystem.h"
#include "Engine/Components/DirectionLightComponent.h"
#include "Engine/Components/TransformComponent.h"

void DirectionLightSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [directionLightPool, transformPool] = registry->GetPools<DirectionLightComponent, TransformComponent>();
	if (!directionLightPool || !transformPool)
		return;

	std::for_each(std::execution::seq, directionLightPool->GetDenseIndices().begin(), directionLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& directionLightComponent = directionLightPool->GetData(entity);

			[[unlikely]]
			if (directionLightPool->IsBitSet<REGENERATE_BIT>(entity))
			{
				directionLightComponent.shadow.version++;
				directionLightPool->ResetBit<REGENERATE_BIT>(entity);
			}

			[[unlikely]]
			if (directionLightComponent.shadow.frameBuffers[frameIndex].version != directionLightComponent.shadow.version)
			{
				directionLightComponent.shadow.frameBuffers[frameIndex].version = directionLightComponent.shadow.version;

				Vk::ImageSpecification depthImageSpec;
				depthImageSpec.type = VK_IMAGE_TYPE_2D;
				depthImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
				depthImageSpec.format = VK_FORMAT_D32_SFLOAT;
				depthImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
				depthImageSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				depthImageSpec.aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
				depthImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

				Vk::FrameBufferBuilder frameBufferBuilder;
				frameBufferBuilder
					.SetSize(directionLightComponent.shadow.textureSize, directionLightComponent.shadow.textureSize)
					.AddDepthSpecification(0, depthImageSpec);

				directionLightComponent.shadow.frameBuffers[frameIndex].frameBuffer = frameBufferBuilder.BuildDynamic();

				//TODO: DYNAMIC DESCRIPTOR ARRAY UPDATE
			}
		}
	);

	std::for_each(std::execution::par_unseq, directionLightPool->GetDenseIndices().begin(), directionLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (transformPool->HasComponent(entity) && (directionLightPool->IsBitSet<UPDATE_BIT>(entity) || transformPool->IsBitSet<CHANGED_BIT>(entity)))
			{
				auto& transformComponent = transformPool->GetData(entity);
				auto& directionLightComponent = directionLightPool->GetData(entity);

				directionLightComponent.position = glm::vec3(transformComponent.transform * defaultLightPosition);
				directionLightComponent.direction = glm::normalize(glm::vec3(transformComponent.transform * glm::vec4(defaultDirectionLightDirection, 0.f)));

				//Todo: ViewProj calculation
				//Todo: FarPlane calculation

				directionLightPool->SetBit<CHANGED_BIT>(entity);
				directionLightComponent.version++;
			}
		}
	);
}

void DirectionLightSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto directionLightPool = registry->GetPool<DirectionLightComponent>();

	if (!directionLightPool)
		return;

	std::for_each(std::execution::par_unseq, directionLightPool->GetDenseIndices().begin(), directionLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if (directionLightPool->IsBitSet<CHANGED_BIT>(entity))
				directionLightPool->GetBitset(entity).reset();
		}
	);
}

void DirectionLightSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [directionLightPool] = registry->GetPools<DirectionLightComponent>();

	if (!directionLightPool)
		return;

	auto directionLightComponentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DirectionLightData", frameIndex);
	auto directionLightComponentBufferHandler = static_cast<DirectionLightGPU*>(directionLightComponentBuffer->buffer->GetHandler());

	auto directionLightBillboardBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DirectionLightBillboard", frameIndex);
	auto directionLightBillboardBufferHandler = static_cast<glm::vec4*>(directionLightBillboardBuffer->buffer->GetHandler());

	std::for_each(std::execution::par_unseq, directionLightPool->GetDenseIndices().begin(), directionLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& directionLightComponent = directionLightPool->GetData(entity);
			auto directionLightIndex = directionLightPool->GetDenseIndex(entity);

			[[unlikely]]
			if (directionLightComponentBuffer->versions[directionLightIndex] != directionLightComponent.version)
			{
				directionLightComponentBuffer->versions[directionLightIndex] = directionLightComponent.version;
				directionLightComponentBufferHandler[directionLightIndex] = DirectionLightGPU(directionLightComponent);
			}

			[[unlikely]]
			if (directionLightBillboardBuffer->versions[directionLightIndex] != directionLightComponent.version)
			{
				directionLightBillboardBuffer->versions[directionLightIndex] = directionLightComponent.version;
				directionLightBillboardBufferHandler[directionLightIndex] = glm::vec4(directionLightComponent.position, entity);
			}
		}
	);
}
