#include "AnimationSystem.h"

#include "Engine/Components/ModelComponent.h"
#include "Engine/Components/AnimationComponent.h"

void AnimationSystem::OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime)
{
	auto [animationPool, modelPool] = registry->GetPools<AnimationComponent, ModelComponent>();
	if (!animationPool || !modelPool)
		return;

	std::for_each(std::execution::seq, animationPool->GetDenseIndices().begin(), animationPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			auto& animationComponent = animationPool->GetData(entity);

			if (animationComponent.animation && animationComponent.animation->state == LoadState::Ready)
			{
				[[unlikely]]
				if (animationPool->IsBitSet<REGENERATE_BIT>(entity))
				{
					animationComponent.nodeTransformBuffers.version++;
					animationPool->ResetBit<REGENERATE_BIT>(entity);
				}

				[[unlikely]]
				if (animationComponent.nodeTransformBuffers.buffers[frameIndex].version != animationComponent.nodeTransformBuffers.version)
				{
					animationComponent.nodeTransformBuffers.buffers[frameIndex].version = animationComponent.nodeTransformBuffers.version;

					uint32_t nodeCount = animationPool->GetData(entity).animation->GetNodeProcessInfo().size();

					animationComponent.nodeTransforms.clear();
					animationComponent.nodeTransforms.resize(nodeCount);

					VkDeviceSize nodeBufferSize = sizeof(NodeTransform) * nodeCount;

					Vk::BufferConfig nodeBufferConfig;
					nodeBufferConfig.size = nodeBufferSize;
					nodeBufferConfig.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
					nodeBufferConfig.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

					animationComponent.nodeTransformBuffers.buffers[frameIndex].buffer = std::make_shared<Vk::Buffer>(nodeBufferConfig);
					animationComponent.nodeTransformBuffers.buffers[frameIndex].buffer->MapMemory();
				}
			}
		}
	);

	std::for_each(std::execution::par, animationPool->GetDenseIndices().begin(), animationPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (animationPool->GetData(entity).animation &&
				animationPool->GetData(entity).animation->state == LoadState::Ready &&
				modelPool->HasComponent(entity) && 	
				modelPool->GetData(entity).model && 
				modelPool->GetData(entity).model->state == LoadState::Ready &&
				(true || animationPool->IsBitSet<UPDATE_BIT>(entity)))
			{
				auto& animationComponent = animationPool->GetData(entity);
				auto& modelComponent = modelPool->GetData(entity);
				animationComponent.time += animationComponent.animation->GetTicksPerSeconds() * deltaTime * animationComponent.speed;
				animationComponent.time = fmod(animationComponent.time, animationComponent.animation->GetDuration());

				auto& boneProcesInfos = animationComponent.animation->GetBoneProcessInfo();
				auto& nodeProcessInfos = animationComponent.animation->GetNodeProcessInfo();
				auto& nodeTransformInfos = modelComponent.model->GetNodeTransformInfos();
				for (uint32_t i = 0; i < nodeProcessInfos.size(); ++i)
				{
					animationComponent.nodeTransforms[i].localTransform = nodeTransformInfos[i].localTransform;

					if (nodeProcessInfos[i].boneIndex != UINT32_MAX)
					{
						auto& boneProcessInfo = boneProcesInfos[nodeProcessInfos[i].boneIndex];
						animationComponent.nodeTransforms[i].localTransform = boneProcessInfo.bone.GetTransform(animationComponent.time);
						animationComponent.nodeTransforms[i].nodeTransform.transform = animationComponent.nodeTransforms[i].localTransform * boneProcessInfo.offsetMatrix;
					}

					if (nodeProcessInfos[i].parentIndex != UINT32_MAX)
					{
						auto& parentTransform = animationComponent.nodeTransforms[nodeProcessInfos[i].parentIndex].localTransform;
						animationComponent.nodeTransforms[i].localTransform = parentTransform * animationComponent.nodeTransforms[i].localTransform;
						animationComponent.nodeTransforms[i].nodeTransform.transform = parentTransform * animationComponent.nodeTransforms[i].nodeTransform.transform;
					}
					
					animationComponent.nodeTransforms[i].nodeTransform.transformIT = glm::transpose(glm::inverse(animationComponent.nodeTransforms[i].nodeTransform.transform));
				}

				animationPool->ResetBit<UPDATE_BIT>(entity);
				animationPool->SetBit<CHANGED_BIT>(entity);
			}
		}
	);
}

void AnimationSystem::OnFinish(std::shared_ptr<Registry> registry)
{
	auto animationPool = registry->GetPool<AnimationComponent>();

	if (!animationPool)
		return;

	std::for_each(std::execution::par, animationPool->GetDenseIndices().begin(), animationPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			[[unlikely]]
			if (animationPool->IsBitSet<CHANGED_BIT>(entity))
				animationPool->GetBitset(entity).reset();
		}
	);
}

void AnimationSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto [animationPool, modelPool] = registry->GetPools<AnimationComponent, ModelComponent>();

	if (!animationPool || !modelPool)
		return;

	auto animationNodeTransformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("AnimationNodeTransformDeviceAddressesBuffers", frameIndex);
	auto animationNodeTransformBufferHandler = static_cast<VkDeviceAddress*>(animationNodeTransformBuffer->buffer->GetHandler());

	std::for_each(std::execution::par, animationPool->GetDenseIndices().begin(), animationPool->GetDenseIndices().end(),
		[&](const Entity& entity) -> void {
			if (animationPool->GetData(entity).animation &&
				animationPool->GetData(entity).animation->state == LoadState::Ready &&
				modelPool->HasComponent(entity) &&
				modelPool->GetData(entity).model &&
				modelPool->GetData(entity).model->state == LoadState::Ready)
			{
				auto& animationComponent = animationPool->GetData(entity);
				auto animationIndex = animationPool->GetDenseIndex(entity);		

				NodeTransform* animationBufferHandler = static_cast<NodeTransform*>(animationComponent.nodeTransformBuffers.buffers[frameIndex].buffer->GetHandler());

				for (uint32_t i = 0; i < animationComponent.nodeTransforms.size(); ++i)
					animationBufferHandler[i] = animationComponent.nodeTransforms[i].nodeTransform;

				[[unlikely]]
				if (animationNodeTransformBuffer->versions[animationIndex] != animationComponent.nodeTransformBuffers.version)
				{
					animationNodeTransformBuffer->versions[animationIndex] = animationComponent.nodeTransformBuffers.version;
					animationNodeTransformBufferHandler[animationIndex] = animationComponent.nodeTransformBuffers.buffers[frameIndex].buffer->GetAddress();
				}
			}
		}
	);
}
