#include "PointLightShadowSystem.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/TransformComponent.h"

void PointLightShadowSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [pointLightPool, pointLightShadowPool] = registry->GetPools<PointLightComponent, PointLightShadowComponent>();
	if (!pointLightPool || !pointLightShadowPool)
		return;

	static const glm::vec3 directions[6] = {
		{ 1.0,  0.0,  0.0 },
		{-1.0,  0.0,  0.0 },
		{ 0.0,  1.0,  0.0 },
		{ 0.0, -1.0,  0.0 },
		{ 0.0,  0.0,  1.0 },
		{ 0.0,  0.0, -1.0 }
	};

	static const glm::vec3 upVectors[6] = {
		{ 0.0, -1.0,  0.0 },
		{ 0.0, -1.0,  0.0 },
		{ 0.0,  0.0,  1.0 },
		{ 0.0,  0.0, -1.0 },
		{ 0.0, -1.0,  0.0 },
		{ 0.0, -1.0,  0.0 }
	};

	std::for_each(std::execution::seq, pointLightShadowPool->GetDenseIndices().begin(), pointLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& pointLightShadowComponent = pointLightShadowPool->GetData(entity);
			auto pointLightShadowDenseIndex = pointLightShadowPool->GetDenseIndex(entity);

			[[unlikely]]
			if (pointLightShadowPool->IsBitSet<REGENERATE_BIT>(entity))
			{
				pointLightShadowComponent.version++;
				pointLightShadowPool->ResetBit<REGENERATE_BIT>(entity);
			}

			//TODO: spotLightShadowComponent SHADOW VERSION -> Other version index for better control?

			[[unlikely]]
			if (pointLightShadowComponent.frameBuffers[frameIndex].version != pointLightShadowComponent.version)
			{
				pointLightShadowComponent.frameBuffers[frameIndex].version = pointLightShadowComponent.version;

				Vk::ImageSpecification depthImageSpec;
				depthImageSpec.type = VK_IMAGE_TYPE_2D;
				depthImageSpec.format = VK_FORMAT_D32_SFLOAT;
				depthImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
				depthImageSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				depthImageSpec.aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
				depthImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				depthImageSpec.arrayLayers = 6;
				depthImageSpec.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
				depthImageSpec.AddImageViewConfig("Default", VK_IMAGE_VIEW_TYPE_CUBE);

				Vk::FrameBufferBuilder frameBufferBuilder;
				frameBufferBuilder
					.SetSize(pointLightShadowComponent.textureSize, pointLightShadowComponent.textureSize)
					.AddDepthSpecification(0, depthImageSpec);

				auto fbo = frameBufferBuilder.BuildDynamic();
				pointLightShadowComponent.frameBuffers[frameIndex].frameBuffer = fbo;

				resourceManager
					->GetVulkanManager()
					->GetDescriptorSet("ShadowDescriptorSet")
					->UpdateImageArrayElement(
						"PointShadowMaps",
						fbo->GetImage("Depth")->GetImageView("Default"),
						VK_NULL_HANDLE,
						pointLightShadowDenseIndex
					);

				std::cout << "Regenerated point light shadow framebuffer for entity " << entity << std::endl;
			}
		}
	);

	std::for_each(std::execution::par, pointLightShadowPool->GetDenseIndices().begin(), pointLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (pointLightPool->HasComponent(entity) && (pointLightPool->IsBitSet<CHANGED_BIT>(entity) || pointLightShadowPool->IsBitSet<UPDATE_BIT>(entity)))
			{
				auto& pointLightComponent = pointLightPool->GetData(entity);
				auto& pointLightShadowComponent = pointLightShadowPool->GetData(entity);
				
				pointLightShadowComponent.farPlane = pointLightComponent.radius;
					
				auto& pos = pointLightComponent.position;

				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.f, pointLightShadowComponent.nearPlane, pointLightComponent.radius);
				shadowProj[1][1] *= -1; //Invert Y for Vulkan

				for (int i = 0; i < 6; ++i)
					pointLightShadowComponent.viewProj[i] = shadowProj * glm::lookAt(pos, pos + directions[i], upVectors[i]);

				pointLightShadowComponent.version++;
				pointLightShadowPool->SetBit<CHANGED_BIT>(entity);
			}
		}
	);
}

void PointLightShadowSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto pointLightShadowPool = registry->GetPool<PointLightShadowComponent>();

	if (!pointLightShadowPool)
		return;

	std::for_each(std::execution::par, pointLightShadowPool->GetDenseIndices().begin(), pointLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if (pointLightShadowPool->IsBitSet<CHANGED_BIT>(entity))
				pointLightShadowPool->GetBitset(entity).reset();
		}
	);
}

void PointLightShadowSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightShadowPool = registry->GetPool<PointLightShadowComponent>();

	if (!pointLightShadowPool)
		return;

	auto shadowBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightShadowData", frameIndex);
	auto shadowBufferHandler = static_cast<PointLightShadowGPU*>(shadowBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, pointLightShadowPool->GetDenseIndices().begin(), pointLightShadowPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& pointLightShadowComponent = pointLightShadowPool->GetData(entity);
			auto pointLightShadowIndex = pointLightShadowPool->GetDenseIndex(entity);

			[[unlikely]]
			if (shadowBuffer->versions[pointLightShadowIndex] != pointLightShadowComponent.version)
			{
				shadowBuffer->versions[pointLightShadowIndex] = pointLightShadowComponent.version;
				shadowBufferHandler[pointLightShadowIndex] = PointLightShadowGPU(pointLightShadowComponent);
			}
		}
	);
}
