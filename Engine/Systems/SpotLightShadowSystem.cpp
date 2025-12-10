#include "SpotLightShadowSystem.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Components/TransformComponent.h"

void SpotLightShadowSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [spotLightPool, spotLightShadowPool] = registry->GetPools<SpotLightComponent, SpotLightShadowComponent>();
	if (!spotLightPool || !spotLightShadowPool)
		return;

	std::for_each(std::execution::seq, spotLightShadowPool->GetDenseIndices().begin(), spotLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& spotLightShadowComponent = spotLightShadowPool->GetData(entity);

			[[unlikely]]
			if (spotLightShadowPool->IsBitSet<REGENERATE_BIT>(entity))
			{
				spotLightShadowComponent.version++;
				spotLightShadowPool->ResetBit<REGENERATE_BIT>(entity);
			}

			[[unlikely]]
			if (spotLightShadowComponent.frameBuffers[frameIndex].version != spotLightShadowComponent.version)
			{
				spotLightShadowComponent.frameBuffers[frameIndex].version = spotLightShadowComponent.version;

				Vk::ImageSpecification depthImageSpec;
				depthImageSpec.type = VK_IMAGE_TYPE_2D;
				depthImageSpec.format = VK_FORMAT_D32_SFLOAT;
				depthImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
				depthImageSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				depthImageSpec.aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
				depthImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				depthImageSpec.AddImageViewConfig("Default", VK_IMAGE_VIEW_TYPE_2D);

				Vk::FrameBufferBuilder frameBufferBuilder;
				frameBufferBuilder
					.SetSize(spotLightShadowComponent.textureSize, spotLightShadowComponent.textureSize)
					.AddDepthSpecification(0, depthImageSpec);

				spotLightShadowComponent.frameBuffers[frameIndex].frameBuffer = frameBufferBuilder.BuildDynamic();

				std::cout << "Regenerated spot light shadow framebuffer for entity " << entity << std::endl;

				//TODO: DYNAMIC DESCRIPTOR ARRAY UPDATE
			}
		}
	);

	std::for_each(std::execution::par, spotLightShadowPool->GetDenseIndices().begin(), spotLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (spotLightPool->HasComponent(entity) && (spotLightPool->IsBitSet<CHANGED_BIT>(entity) || spotLightShadowPool->IsBitSet<UPDATE_BIT>(entity)))
			{
				auto& spotLightComponent = spotLightPool->GetData(entity);
				auto& spotLightShadowComponent = spotLightShadowPool->GetData(entity);

				spotLightShadowComponent.farPlane = spotLightComponent.range;

				glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
				if (glm::abs(glm::dot(spotLightComponent.direction, up)) > 0.99f) {
					up = glm::vec3(1.f, 0.f, 0.f);
				}

				glm::mat4 view = glm::lookAt(spotLightComponent.position, spotLightComponent.position + spotLightComponent.direction, up);
				glm::mat4 proj = glm::perspective(glm::radians(spotLightComponent.angles.y * 2), 1.f, spotLightShadowComponent.nearPlane, spotLightShadowComponent.farPlane);
				spotLightShadowComponent.viewProj = proj * view;
				spotLightShadowComponent.viewProj[1][1] *= -1; //Invert Y for Vulkan

				spotLightShadowComponent.version++;
				spotLightShadowPool->SetBit<CHANGED_BIT>(entity);
			}
		}
	);
}

void SpotLightShadowSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto spotLightShadowPool = registry->GetPool<SpotLightShadowComponent>();

	if (!spotLightShadowPool)
		return;

	std::for_each(std::execution::par, spotLightShadowPool->GetDenseIndices().begin(), spotLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if (spotLightShadowPool->IsBitSet<CHANGED_BIT>(entity))
				spotLightShadowPool->GetBitset(entity).reset();
		}
	);
}

void SpotLightShadowSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightShadowPool = registry->GetPool<SpotLightShadowComponent>();

	if (!spotLightShadowPool)
		return;

	auto shadowBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightShadowData", frameIndex);
	auto shadowBufferHandler = static_cast<SpotLightShadowGPU*>(shadowBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, spotLightShadowPool->GetDenseIndices().begin(), spotLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& spotLightShadowComponent = spotLightShadowPool->GetData(entity);
			auto spotLightShadowIndex = spotLightShadowPool->GetDenseIndex(entity);

			[[unlikely]]
			if (shadowBuffer->versions[spotLightShadowIndex] != spotLightShadowComponent.version)
			{
				shadowBuffer->versions[spotLightShadowIndex] = spotLightShadowComponent.version;
				shadowBufferHandler[spotLightShadowIndex] = SpotLightShadowGPU(spotLightShadowComponent);
			}
		}
	);
}
