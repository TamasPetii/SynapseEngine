#include "Materialized.h"
#include "Engine/Managers/MaterialManager.h"
#include <algorithm>
#include <execution>
#include <ranges>

std::shared_ptr<Vk::Buffer> Materialized::GetMaterialBuffer()
{
    return materialBuffer;
}

void Materialized::UploadMaterialDataToGpu(std::shared_ptr<MaterialManager> materialManager)
{
    //No materials found, or at least 1 mesh had no materials
    if (materials.empty())
        materials.push_back(materialManager->GetMaterial("Default"));

    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * materials.size();

        Vk::BufferConfig config;
        config.size = bufferSize;
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        materialBuffer = std::make_shared<Vk::Buffer>(config);
        materialBuffer->MapMemory();

        auto materialBufferHandler = static_cast<uint32_t*>(materialBuffer->GetHandler());

        for (int i = 0; i < materials.size(); ++i)
        {
            if (materials[i] == nullptr)
                materialBufferHandler[i] = materialManager->GetMaterial("Default")->GetBufferArrayIndex();
            else
                materialBufferHandler[i] = materials[i]->GetBufferArrayIndex();
        }

        materialBuffer->UnmapMemory();
    }
}
