#include "SpotLightSystem.h"
#include "Engine/Config.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Components/TransformComponent.h"

void SpotLightSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [spotLightPool, transformPool, spotLightShadowPool] = registry->GetPools<SpotLightComponent, TransformComponent, SpotLightShadowComponent>();
	if (!spotLightPool || !transformPool)
		return;

	std::for_each(std::execution::seq, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {

			if (spotLightPool->IsBitSet<UPDATE_BIT>(entity))
			{
				auto& spotLightComponent = spotLightPool->GetData(entity);

				bool hasShadowComp = spotLightShadowPool ? spotLightShadowPool->HasComponent(entity) : false;

				if (spotLightComponent.useShadow && !hasShadowComp)
				{
					registry->AddComponents<SpotLightShadowComponent>(entity);
				}
				else if (!spotLightComponent.useShadow && hasShadowComp)
				{
					registry->RemoveComponents<SpotLightShadowComponent>(entity);
				}
			}
		}
	);

	//Todo: Not a good idea to connect transform to spotlight... 

	std::for_each(std::execution::par, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
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

				spotLightComponent.range = scale.x;
				spotLightComponent.angles.x = scale.y;
				spotLightComponent.angles.y = scale.z;
				spotLightComponent.angles.z = glm::cos(glm::radians(spotLightComponent.angles.x));
				spotLightComponent.angles.w = glm::cos(glm::radians(spotLightComponent.angles.y));

				float scaleY = 0.5 * spotLightComponent.range;
				float scaleXZ = glm::tan(glm::radians(spotLightComponent.angles.y)) * scaleY * 2;

				spotLightComponent.transform = glm::inverse(glm::lookAt<float>(spotLightComponent.position, spotLightComponent.position + glm::normalize(spotLightComponent.direction), GlobalConfig::World::up));
				spotLightComponent.transform = glm::rotate(spotLightComponent.transform, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
				spotLightComponent.transform = glm::scale(spotLightComponent.transform, glm::vec3(scaleXZ, scaleY, scaleXZ));
				spotLightComponent.transform = glm::translate(spotLightComponent.transform, glm::vec3(0.f, -1.f, 0.f));

				//Calculating bounding sphere
				if (glm::radians(spotLightComponent.angles.y) > glm::pi<float>() / 4.0f)
				{
					spotLightComponent.boundingSphereOrigin = spotLightComponent.position + spotLightComponent.angles.w * spotLightComponent.range * spotLightComponent.direction;
					spotLightComponent.boundingSphereRadius = glm::sin(glm::radians(spotLightComponent.angles.y)) * spotLightComponent.range;
				}
				else
				{
					spotLightComponent.boundingSphereOrigin = spotLightComponent.position + spotLightComponent.range / (2.0f * spotLightComponent.angles.w) * spotLightComponent.direction;
					spotLightComponent.boundingSphereRadius = spotLightComponent.range / (2.0f * spotLightComponent.angles.w);
				}	

				//Calculate bounding aabb
				glm::vec3 ref = GlobalConfig::World::up;
				glm::vec3 right = glm::cross(ref, spotLightComponent.direction);

				if (glm::length(right) < 0.001f) {
					glm::vec3 alt_ref = glm::vec3(1.0f, 0.0f, 0.0f);
					right = glm::cross(alt_ref, spotLightComponent.direction);
				}

				right = glm::normalize(right);
				glm::vec3 local_up = glm::normalize(glm::cross(spotLightComponent.direction, right));
				float radius = spotLightComponent.range * glm::tan(glm::radians(spotLightComponent.angles.y));
				glm::vec3 baseCenter = spotLightComponent.position + spotLightComponent.direction * spotLightComponent.range;

				std::array<glm::vec3, 5> points =
				{
					spotLightComponent.position,
					baseCenter + local_up * radius,
					baseCenter + -local_up * radius,
					baseCenter + right * radius,
					baseCenter + -right * radius
				};

				glm::vec3 maxPosition{ std::numeric_limits<float>::lowest() };
				glm::vec3 minPosition{ std::numeric_limits<float>::max() };

				for (const auto& point : points)
				{
					maxPosition = glm::max(maxPosition, point);
					minPosition = glm::min(minPosition, point);
				}

				spotLightComponent.aabbOrigin = 0.5f * (minPosition + maxPosition);
				spotLightComponent.aabbExtents = 0.5f * (maxPosition - minPosition);

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

	std::for_each(std::execution::par, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
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
	auto [spotLightPool, spotLightShadowPool] = registry->GetPools<SpotLightComponent, SpotLightShadowComponent>();
	if (!spotLightPool)
		return;

	auto spotLightComponentBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex);
	auto spotLightComponentBufferHandler = static_cast<SpotLightGPU*>(spotLightComponentBuffer->buffer->GetHandler());

	auto spotLightTransformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightTransform", frameIndex);
	auto spotLightTransformBufferHandler = static_cast<glm::mat4*>(spotLightTransformBuffer->buffer->GetHandler());

	auto spotLightBoundingSphereTransformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightBoundingSphereTransform", frameIndex);
	auto spotLightBoundingSphereTransformBufferHandler = static_cast<glm::mat4*>(spotLightBoundingSphereTransformBuffer->buffer->GetHandler());

	auto spotLightBoundingBoxTransformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightBoundingBoxTransform", frameIndex);
	auto spotLightBoundingBoxTransformBufferHandler = static_cast<glm::mat4*>(spotLightBoundingBoxTransformBuffer->buffer->GetHandler());

	auto spotLightBillboardBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightBillboard", frameIndex);
	auto spotLightBillboardBufferHandler = static_cast<glm::vec4*>(spotLightBillboardBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, spotLightPool->GetDenseIndices().begin(), spotLightPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& spotLightComponent = spotLightPool->GetData(entity);
			auto spotLightIndex = spotLightPool->GetDenseIndex(entity);

			[[unlikely]]
			if (spotLightComponentBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightComponentBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightComponentBufferHandler[spotLightIndex] = SpotLightGPU(spotLightComponent, spotLightShadowPool && spotLightShadowPool->HasComponent(entity) ? spotLightShadowPool->GetDenseIndex(entity) : UINT32_MAX);

			}

			[[unlikely]]
			if (spotLightTransformBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightTransformBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightTransformBufferHandler[spotLightIndex] = spotLightComponent.transform;
			}

			[[unlikely]]
			if (spotLightBoundingSphereTransformBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightBoundingSphereTransformBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightBoundingSphereTransformBufferHandler[spotLightIndex] = glm::translate(spotLightComponent.boundingSphereOrigin) * glm::scale(glm::vec3(spotLightComponent.boundingSphereRadius));
			}

			[[unlikely]]
			if (spotLightBoundingBoxTransformBuffer->versions[spotLightIndex] != spotLightComponent.version)
			{
				spotLightBoundingBoxTransformBuffer->versions[spotLightIndex] = spotLightComponent.version;
				spotLightBoundingBoxTransformBufferHandler[spotLightIndex] = glm::translate(spotLightComponent.aabbOrigin) * glm::scale(glm::vec3(spotLightComponent.aabbExtents));
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
