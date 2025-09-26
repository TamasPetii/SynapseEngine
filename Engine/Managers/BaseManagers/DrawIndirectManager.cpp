#include "DrawIndirectManager.h"

DrawIndirectManager::DrawIndirectManager()
{
    
}

void DrawIndirectManager::Update(uint32_t frameIndex, uint32_t count)
{
    constexpr uint32_t instanceBlockSize = 16;

    uint32_t requiredSize = static_cast<uint32_t>(std::ceil(count / (float)instanceBlockSize)) * instanceBlockSize;

    if (instanceIndexBuffers[frameIndex].first == nullptr || instanceIndexBuffers[frameIndex].second != requiredSize)
    {
        instanceIndexBuffers[frameIndex].second = requiredSize;

        Vk::BufferConfig config;
        config.size = requiredSize * sizeof(uint32_t);
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        instanceIndexBuffers[frameIndex] = std::make_pair(std::make_shared<Vk::Buffer>(config), requiredSize);
        instanceIndexBuffers[frameIndex].first->MapMemory();
    }

    if (indirectCommandBuffers[frameIndex].first == nullptr || indirectCommandBuffers[frameIndex].second != requiredSize)
    {
        indirectCommandBuffers[frameIndex].second = requiredSize;

        Vk::BufferConfig config;
        config.size = requiredSize * sizeof(VkDrawIndirectCommand);
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        indirectCommandBuffers[frameIndex] = std::make_pair(std::make_shared<Vk::Buffer>(config), requiredSize);
        indirectCommandBuffers[frameIndex].first->MapMemory();
    }
}
