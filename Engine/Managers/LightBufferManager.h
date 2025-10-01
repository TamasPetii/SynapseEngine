#pragma once
#include <array>
#include "Engine/EngineApi.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Config.h"

#include "BaseManagers/DynamicSizeBuffer.h"
#include "Engine/Utils/VersionedObject.h"
#include "Engine/Registry/Registry.h"

template <typename L>
class LightBufferManager
{
public:
	LightBufferManager();
	void Update(std::shared_ptr<Registry> registry, uint32_t frameIndex);
private:
	void InitLightResources();
private:
	template<typename T>
	void UpdateBuffer(std::shared_ptr<DynamicSizeBuffer> dynamicBuffer, uint32_t requiredSize);
private:
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> lightIndirectDrawBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> lightInstanceIndexBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> lightShadowCountBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> lightShadowInstanceIndexBuffers;
};

template<typename L>
LightBufferManager<L>::LightBufferManager()
{
	InitLightResources();
}

template<typename L>
void LightBufferManager<L>::InitLightResources()
{
	for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
	{
		lightIndirectDrawBuffers[i] = std::make_shared<DynamicSizeBuffer>();
		lightShadowCountBuffers[i] = std::make_shared<DynamicSizeBuffer>();
		lightInstanceIndexBuffers[i] = std::make_shared<DynamicSizeBuffer>();
		lightShadowInstanceIndexBuffers[i] = std::make_shared<DynamicSizeBuffer>();

		lightShadowCountBuffers[i]->size = 1;
		UpdateBuffer<uint32_t>(lightShadowCountBuffers[i], 1);

		lightIndirectDrawBuffers[i]->size = 1;
		UpdateBuffer<VkDrawIndirectCommand>(lightIndirectDrawBuffers[i], 1);
	}
}

template<typename L>
template<typename T>
void LightBufferManager<L>::UpdateBuffer(std::shared_ptr<DynamicSizeBuffer> dynamicBuffer, uint32_t requiredSize)
{
	if (dynamicBuffer->buffer == nullptr || dynamicBuffer->size != requiredSize)
	{
		Vk::BufferConfig config;
		config.size = requiredSize * sizeof(T);
		config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
		config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		dynamicBuffer->size = requiredSize;
		dynamicBuffer->buffer = std::make_shared<Vk::Buffer>(config);
		dynamicBuffer->buffer->MapMemory();
		dynamicBuffer->version++;
	}
}

template<typename L>
void LightBufferManager<L>::Update(std::shared_ptr<Registry> registry, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<L>();

	uint32_t count = pointLightPool ? pointLightPool->GetDenseSize() : 0;
	uint32_t baseBufferSize = GlobalConfig::BufferConfig::lightInstanceSize;
	uint32_t requiredSize = static_cast<uint32_t>(std::ceil((count + 1) / (float)baseBufferSize)) * baseBufferSize;

	UpdateBuffer<uint32_t>(lightInstanceIndexBuffers[frameIndex], requiredSize);
	UpdateBuffer<uint32_t>(lightShadowInstanceIndexBuffers[frameIndex], requiredSize);
}