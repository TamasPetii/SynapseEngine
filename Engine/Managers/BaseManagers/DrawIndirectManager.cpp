#include "DrawIndirectManager.h"

DrawIndirectManager::DrawIndirectManager()
{
    for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
    {
        indirectCommandBuffers[i] = std::make_shared<DynamicSizeBuffer>();
        instanceIndexAddressBuffers[i] = std::make_shared<DynamicSizeBuffer>();

        Vk::BufferConfig config;
        config.size = sizeof(uint32_t);
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        indirectCountBuffers[i] = std::make_shared<Vk::Buffer>(config);
        indirectCountBuffers[i]->MapMemory();
    }
}

void DrawIndirectManager::Update(uint32_t frameIndex, uint32_t count, uint32_t bufferBaseSize)
{
    uint32_t requiredSize = static_cast<uint32_t>(std::ceil((count + 1) / (float)bufferBaseSize)) * bufferBaseSize;

    { //Instance Address Buffer and Instance Index Buffers

        if (instanceIndexAddressBuffers[frameIndex]->buffer == nullptr || instanceIndexAddressBuffers[frameIndex]->size != requiredSize)
        {
            Vk::BufferConfig config;
            config.size = requiredSize * sizeof(VkDeviceAddress);
            config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
            config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            instanceIndexAddressBuffers[frameIndex]->size = requiredSize;
            instanceIndexAddressBuffers[frameIndex]->buffer = std::make_shared<Vk::Buffer>(config);
            instanceIndexAddressBuffers[frameIndex]->buffer->MapMemory();
            instanceIndexAddressBuffers[frameIndex]->version++;

            std::cout << std::format("[DrawIndirectManager::Update] - instanceIndexAddressBuffers resized {} in frame {}", requiredSize, frameIndex) << std::endl;
        }

        { //Instance Index Buffers
            if (instanceIndexBuffers[frameIndex].size() != requiredSize)
                instanceIndexBuffers[frameIndex].resize(requiredSize);

            for (uint32_t i = 0; i < requiredSize; ++i)
            {
                if (instanceIndexBuffers[frameIndex][i] == nullptr)
                {
                    instanceIndexBuffers[frameIndex][i] = std::make_shared<VersionedObject<DynamicSizeBuffer>>(std::make_shared<DynamicSizeBuffer>());

                    //std::cout << std::format("[DrawIndirectManager] - VersionedObject<IndexBuffer> created {} in frame {} for model {}", requiredSize, frameIndex, i) << std::endl;
                }
            }
        }
    }

    { //Draw Indirect Command Buffer
        if (indirectCommandBuffers[frameIndex]->buffer == nullptr || indirectCommandBuffers[frameIndex]->size != requiredSize)
        {
            Vk::BufferConfig config;
            config.size = requiredSize * sizeof(VkDrawIndirectCommand);
            config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT;
            config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            indirectCommandBuffers[frameIndex]->size = requiredSize;
            indirectCommandBuffers[frameIndex]->buffer = std::make_shared<Vk::Buffer>(config);
            indirectCommandBuffers[frameIndex]->buffer->MapMemory();
            indirectCommandBuffers[frameIndex]->version++;

            for (uint32_t i = 0; i < requiredSize; ++i)
            {
                static_cast<VkDrawIndirectCommand*>(indirectCommandBuffers[frameIndex]->buffer->GetHandler())[i] = VkDrawIndirectCommand{
                    .vertexCount = 0,
                    .instanceCount = 0,
                    .firstVertex = 0,
                    .firstInstance = 0
                };
            }

            std::cout << std::format("[DrawIndirectManager::Update] - indirectCommandBuffers resized {} in frame {}", requiredSize, frameIndex) << std::endl;
        }
    }
}

void DrawIndirectManager::UpdateInstanceBuffer(uint32_t frameIndex, uint32_t bufferIndex, uint32_t count)
{
    { //Instance Index Buffers
        uint32_t baseBufferSize = GlobalConfig::BufferConfig::instanceBufferBaseSize;
        uint32_t requiredSize = static_cast<uint32_t>(std::ceil((count + 1) / (float)baseBufferSize)) * baseBufferSize;

        auto dynamicSizedBuffer = instanceIndexBuffers[frameIndex][bufferIndex]->object;

        bool bufferRegenerated = false;

        if (dynamicSizedBuffer->buffer == nullptr || dynamicSizedBuffer->size != requiredSize)
        {
            Vk::BufferConfig config;
            config.size = requiredSize * sizeof(uint32_t);
            config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
            config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            dynamicSizedBuffer->size = requiredSize;
            dynamicSizedBuffer->buffer = std::make_shared<Vk::Buffer>(config);
            dynamicSizedBuffer->buffer->MapMemory();
            dynamicSizedBuffer->version++;

            bufferRegenerated = true;

            std::cout << std::format("[DrawIndirectManager::UpdateInstanceBuffer] - instanceIndexBuffer resized {} in frame {} for model {}", requiredSize, frameIndex, bufferIndex) << std::endl;
        }

        //InstanceIndexAddressBuffer resized and regenerated -> Need to reupload all exsisting buffer addresses!
        if (bufferRegenerated || instanceIndexBuffers[frameIndex][bufferIndex]->versions[frameIndex] != instanceIndexAddressBuffers[frameIndex]->version)
        {
            //Its not a problem if instanceIndexBuffers changes, version is not increased! 
            instanceIndexBuffers[frameIndex][bufferIndex]->versions[frameIndex] = instanceIndexAddressBuffers[frameIndex]->version;

            auto bufferHandler = static_cast<VkDeviceAddress*>(instanceIndexAddressBuffers[frameIndex]->buffer->GetHandler());
            bufferHandler[bufferIndex] = dynamicSizedBuffer->buffer->GetAddress();

            std::cout << std::format("[DrawIndirectManager::UpdateInstanceBuffer] - instanceIndexBuffer address uploaded in frame {} for model {}", frameIndex, bufferIndex) << std::endl;
        }
    }
}
