#include "LightBufferManager.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"
#include <iostream>
#include <format>

LightBufferManager::LightBufferManager()
{
    for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
    {
        pointLightInstanceIndexBuffers[i] = std::make_shared<DynamicSizeBuffer>();
        pointLightShadowInstanceIndexBuffers[i] = std::make_shared<DynamicSizeBuffer>();

        spotLightInstanceIndexBuffers[i] = std::make_shared<DynamicSizeBuffer>();
        spotLightShadowInstanceIndexBuffers[i] = std::make_shared<DynamicSizeBuffer>();
    }
}

void LightBufferManager::Update(std::shared_ptr<Registry> registry, uint32_t frameIndex)
{
    UpdatePointLightResources(registry, frameIndex, GlobalConfig::BufferConfig::pointLightInstanceSize);
    UpdateSpotLightResources(registry, frameIndex, GlobalConfig::BufferConfig::spotLightInstanceSize);
}

void LightBufferManager::UpdateBuffer(std::shared_ptr<DynamicSizeBuffer> dynamicBuffer, uint32_t requiredSize)
{
    if (dynamicBuffer->buffer == nullptr || dynamicBuffer->size != requiredSize)
    {
        Vk::BufferConfig config;
        config.size = requiredSize * sizeof(uint32_t);
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        dynamicBuffer->size = requiredSize;
        dynamicBuffer->buffer = std::make_shared<Vk::Buffer>(config);
        dynamicBuffer->buffer->MapMemory();
        dynamicBuffer->version++;
    }
}

void LightBufferManager::UpdatePointLightResources(std::shared_ptr<Registry> registry, uint32_t frameIndex, uint32_t baseBufferSize)
{
    auto pointLightPool = registry->GetPool<PointLightComponent>();

    uint32_t count = pointLightPool ? pointLightPool->GetDenseSize() : 0;
    uint32_t requiredSize = static_cast<uint32_t>(std::ceil((count + 1) / (float)baseBufferSize)) * baseBufferSize;

    UpdateBuffer(pointLightInstanceIndexBuffers[frameIndex], requiredSize);
    UpdateBuffer(pointLightShadowInstanceIndexBuffers[frameIndex], requiredSize);
}

void LightBufferManager::UpdateSpotLightResources(std::shared_ptr<Registry> registry, uint32_t frameIndex, uint32_t baseBufferSize)
{
    auto spotLightPool = registry->GetPool<SpotLightComponent>();

    uint32_t count = spotLightPool ? spotLightPool->GetDenseSize() : 0;
    uint32_t requiredSize = static_cast<uint32_t>(std::ceil((count + 1) / (float)baseBufferSize)) * baseBufferSize;

    UpdateBuffer(spotLightInstanceIndexBuffers[frameIndex], requiredSize);
    UpdateBuffer(spotLightShadowInstanceIndexBuffers[frameIndex], requiredSize);
}
