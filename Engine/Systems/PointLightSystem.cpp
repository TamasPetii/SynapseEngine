#include "PointLightSystem.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/TransformComponent.h"

void PointLightSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [pointLightPool, transformPool, pointLightShadowPool] = registry->GetPools<PointLightComponent, TransformComponent, PointLightShadowComponent>();
	if (!pointLightPool || !transformPool)
		return;

	std::for_each(std::execution::seq, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {

			if (pointLightPool->IsBitSet<UPDATE_BIT>(entity))
			{
				auto& pointLightComponent = pointLightPool->GetData(entity);

				bool hasShadowComp = pointLightShadowPool ? pointLightShadowPool->HasComponent(entity) : false;

				if (pointLightComponent.useShadow && !hasShadowComp)
				{
					registry->AddComponents<PointLightShadowComponent>(entity);
				}
				else if (!pointLightComponent.useShadow && hasShadowComp)
				{
					registry->RemoveComponents<PointLightShadowComponent>(entity);
				}
			}
			
			
		}
	);

	std::for_each(std::execution::par, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (transformPool->HasComponent(entity) && (pointLightPool->IsBitSet<UPDATE_BIT>(entity) || transformPool->IsBitSet<CHANGED_BIT>(entity)))
			{
				auto& transformComponent = transformPool->GetData(entity);
				auto& pointLightComponent = pointLightPool->GetData(entity);

				pointLightComponent.position = glm::vec3(transformComponent.transform * defaultLightPosition);

				glm::vec3 scale;
				scale.x = glm::length(glm::vec3(transformComponent.transform[0]));
				scale.y = glm::length(glm::vec3(transformComponent.transform[1]));
				scale.z = glm::length(glm::vec3(transformComponent.transform[2]));

				pointLightComponent.radius = defaultPointLightRadius * std::max({ scale.x, scale.y, scale.z });
				
				pointLightComponent.transform = glm::mat4(1.0f);
				pointLightComponent.transform = glm::translate(pointLightComponent.transform, pointLightComponent.position);
				pointLightComponent.transform = glm::scale(pointLightComponent.transform, glm::vec3(pointLightComponent.radius));

				pointLightPool->SetBit<CHANGED_BIT>(entity);
				pointLightComponent.version++;
			}
		}
	);
}

void PointLightSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();

	if (!pointLightPool)
		return;

	std::for_each(std::execution::par, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			pointLightPool->GetData(entity).toRender = false;

			[[unlikely]]
			if (pointLightPool->IsBitSet<CHANGED_BIT>(entity))
				pointLightPool->GetBitset(entity).reset();
		}
	);
}

void PointLightSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [pointLightPool, pointLightShadowPool] = registry->GetPools<PointLightComponent, PointLightShadowComponent>();
	if (!pointLightPool)
		return;

	auto pointLightComponentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex);
	auto pointLightComponentBufferHandler = static_cast<PointLightGPU*>(pointLightComponentBuffer->buffer->GetHandler());

	auto pointLightTransformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightTransform", frameIndex);
	auto pointLightTransformBufferHandler = static_cast<glm::mat4*>(pointLightTransformBuffer->buffer->GetHandler());

	auto pointLightBillboardBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightBillboard", frameIndex);
	auto pointLightBillboardBufferHandler = static_cast<glm::vec4*>(pointLightBillboardBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, pointLightPool->GetDenseIndices().begin(), pointLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& pointLightComponent = pointLightPool->GetData(entity);
			auto pointLightIndex = pointLightPool->GetDenseIndex(entity);

			[[unlikely]]
			if (pointLightComponentBuffer->versions[pointLightIndex] != pointLightComponent.version)
			{
				pointLightComponentBuffer->versions[pointLightIndex] = pointLightComponent.version;
				pointLightComponentBufferHandler[pointLightIndex] = PointLightGPU(pointLightComponent, pointLightShadowPool && pointLightShadowPool->HasComponent(entity) ? pointLightShadowPool->GetDenseIndex(entity) : UINT32_MAX);
			}

			[[unlikely]]
			if (pointLightTransformBuffer->versions[pointLightIndex] != pointLightComponent.version)
			{
				pointLightTransformBuffer->versions[pointLightIndex] = pointLightComponent.version;
				pointLightTransformBufferHandler[pointLightIndex] = pointLightComponent.transform;
			}

			[[unlikely]]
			if (pointLightBillboardBuffer->versions[pointLightIndex] != pointLightComponent.version)
			{
				pointLightBillboardBuffer->versions[pointLightIndex] = pointLightComponent.version;
				pointLightBillboardBufferHandler[pointLightIndex] = glm::vec4(pointLightComponent.position, entity);
			}
		}
	);
}
