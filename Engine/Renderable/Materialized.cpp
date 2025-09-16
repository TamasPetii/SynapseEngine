#include "Materialized.h"
#include <algorithm>
#include <execution>
#include <ranges>

std::shared_ptr<Vk::Buffer> Materialized::GetMaterialBuffer()
{
    return materialBuffer;
}

void Materialized::UploadMaterialDataToGpu()
{
    //No materials found, or at least 1 mesh had no materials
    if (materials.empty())
    {
        MaterialComponent material;
        material.color = glm::vec4(10, 0, 0, 1);
        materials.push_back(material);
    }
    //TODO: Is it even handled?

    {
        VkDeviceSize bufferSize = sizeof(MaterialComponentGPU) * materials.size();

        Vk::BufferConfig config;
        config.size = bufferSize;
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        materialBuffer = std::make_shared<Vk::Buffer>(config);
        materialBuffer->MapMemory();
        auto materialBufferHandler = static_cast<MaterialComponentGPU*>(materialBuffer->GetHandler());

        const auto timeout = std::chrono::seconds(30);
        const auto checkInterval = std::chrono::milliseconds(100);

        auto vertex_index_range = std::views::iota(0u, (uint32_t)materials.size());
        std::for_each(std::execution::seq, vertex_index_range.begin(), vertex_index_range.end(),
        [&](auto materialIndex) -> void {
            bool albedoReady = false, normalReady = false, metallicReady = false, roughnessReady = false;     
            auto& material = materials[materialIndex];

            auto startTime = std::chrono::steady_clock::now();

            while (true)
            {
                bool albedoReady = !material.albedoTexture || (material.albedoTexture->state == LoadState::Ready || material.albedoTexture->state == LoadState::Failed);
                bool normalReady = !material.normalTexture || (material.normalTexture->state == LoadState::Ready || material.normalTexture->state == LoadState::Failed);
                bool metallicReady = !material.metalnessTexture || (material.metalnessTexture->state == LoadState::Ready || material.metalnessTexture->state == LoadState::Failed);
                bool roughnessReady = !material.roughnessTexture || (material.roughnessTexture->state == LoadState::Ready || material.roughnessTexture->state == LoadState::Failed);
            
                if (albedoReady && normalReady && metallicReady && roughnessReady)
                    break;

                if (std::chrono::steady_clock::now() - startTime > timeout)
                    break;

                std::this_thread::sleep_for(checkInterval);
            }

            materialBufferHandler[materialIndex] = MaterialComponentGPU(material);
        });

        materialBuffer->UnmapMemory();
    }
}
