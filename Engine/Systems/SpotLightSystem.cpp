#include "SpotLightSystem.h"
#include "Engine/Config.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Components/TransformComponent.h"

void SpotLightSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [spotLightPool, transformPool] = registry->GetPools<SpotLightComponent, TransformComponent>();
	if (!spotLightPool || !transformPool)
		return;

	std::for_each(std::execution::seq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& spotLightComponent = spotLightPool->GetData(entity);

			[[unlikely]]
			if (spotLightPool->IsBitSet<REGENERATE_BIT>(entity))
			{
				spotLightComponent.shadow.version++;
				spotLightPool->ResetBit<REGENERATE_BIT>(entity);
			}

			[[unlikely]]
			if (spotLightComponent.shadow.frameBuffers[frameIndex].version != spotLightComponent.shadow.version)
			{
				spotLightComponent.shadow.frameBuffers[frameIndex].version = spotLightComponent.shadow.version;

				/*
				Vk::ImageSpecification depthImageSpec;
				depthImageSpec.type = VK_IMAGE_TYPE_2D;
				depthImageSpec.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				depthImageSpec.format = VK_FORMAT_D32_SFLOAT;
				depthImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
				depthImageSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				depthImageSpec.aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
				depthImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				//VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT -> Need to refactor

				Vk::FrameBufferBuilder frameBufferBuilder;
				frameBufferBuilder
					.SetSize(pointLightComponent.shadow.textureSize, pointLightComponent.shadow.textureSize)
					.AddDepthSpecification(0, depthImageSpec);

				pointLightComponent.shadow.frameBuffers[frameIndex].frameBuffer = frameBufferBuilder.BuildDynamic();
				*/

				//TODO: DYNAMIC DESCRIPTOR ARRAY UPDATE
			}
		}
	);

	std::for_each(std::execution::par_unseq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (transformPool->HasComponent(entity) && (spotLightPool->IsBitSet<UPDATE_BIT>(entity) || transformPool->IsBitSet<CHANGED_BIT>(entity)))
			{
				auto& transformComponent = transformPool->GetData(entity);
				auto& spotLightComponent = spotLightPool->GetData(entity);

				spotLightComponent.position = glm::vec3(transformComponent.transform * defaultLightPosition);
				spotLightComponent.direction = glm::normalize(glm::vec3(transformComponent.transform * glm::vec4(defaultSpotLightDirection, 0.f)));

				glm::vec3 scale;
				scale.x = glm::length(glm::vec3(transformComponent.transform[0]));
				scale.y = glm::length(glm::vec3(transformComponent.transform[1]));
				scale.z = glm::length(glm::vec3(transformComponent.transform[2]));

				spotLightComponent.length = scale.x;
				spotLightComponent.angles.x = scale.y;
				spotLightComponent.angles.y = scale.z;
				spotLightComponent.angles.z = glm::cos(glm::radians(spotLightComponent.angles.x));
				spotLightComponent.angles.w = glm::cos(glm::radians(spotLightComponent.angles.y));

				float scaleY = 0.5 * spotLightComponent.length;
				float scaleXZ = glm::tan(glm::radians(spotLightComponent.angles.y)) * scaleY * 2;

				spotLightComponent.transform = glm::inverse(glm::lookAt<float>(spotLightComponent.position, spotLightComponent.position + glm::normalize(spotLightComponent.direction), GlobalConfig::World::up));
				spotLightComponent.transform = glm::rotate(spotLightComponent.transform, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
				spotLightComponent.transform = glm::scale(spotLightComponent.transform, glm::vec3(scaleXZ, scaleY, scaleXZ));
				spotLightComponent.transform = glm::translate(spotLightComponent.transform, glm::vec3(0.f, -1.f, 0.f));

				//Maybe cone is not good, need to reimplement shapes

				//Todo: ViewProj calculation

				spotLightPool->SetBit<CHANGED_BIT>(entity);
				spotLightComponent.version++;
			}
		}
	);
}

void SpotLightSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();

	if (!spotLightPool)
		return;

	std::for_each(std::execution::par_unseq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			spotLightPool->GetData(entity).toRender = false;

			[[unlikely]]
			if (spotLightPool->IsBitSet<CHANGED_BIT>(entity))
				spotLightPool->GetBitset(entity).reset();
		}
	);
}

void SpotLightSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();

	if (!spotLightPool)
		return;

	auto spotLightComponentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex);
	auto spotLightComponentBufferHandler = static_cast<SpotLightGPU*>(spotLightComponentBuffer->buffer->GetHandler());

	auto spotLightTransformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightTransform", frameIndex);
	auto spotLightTransformBufferHandler = static_cast<glm::mat4*>(spotLightTransformBuffer->buffer->GetHandler());

	auto spotLightBillboardBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightBillboard", frameIndex);
	auto spotLightBillboardBufferHandler = static_cast<glm::vec4*>(spotLightBillboardBuffer->buffer->GetHandler());

	std::for_each(std::execution::par_unseq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& spotLightComponent = spotLightPool->GetData(entity);
			auto spotLightIndex = spotLightPool->GetDenseIndex(entity);

			[[unlikely]]
			if (spotLightComponentBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightComponentBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightComponentBufferHandler[spotLightIndex] = SpotLightGPU(spotLightComponent);
			}

			[[unlikely]]
			if (spotLightTransformBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightTransformBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightTransformBufferHandler[spotLightIndex] = spotLightComponent.transform;
			}

			[[unlikely]]
			if (spotLightBillboardBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightBillboardBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightBillboardBufferHandler[spotLightIndex] = glm::vec4(spotLightComponent.position, entity);
			}
		}
	);
}
