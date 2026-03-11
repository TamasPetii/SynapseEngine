#include "SceneDrawData.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Manager/ModelManager.h"

namespace Syn
{
    SceneDrawData::SceneDrawData(uint32_t frameCount)
    {
        modelAllocations.resize(ModelManager::MAX_MODELS);
        meshAllocations.resize(MAX_INDIRECT_COMMANDS);
        drawDescriptors.resize(MAX_INDIRECT_COMMANDS);
        traditionalCommands.resize(MESHLET_OFFSET_START, { 0,0,0,0 });
        meshletCommands.resize(MAX_INDIRECT_COMMANDS - MESHLET_OFFSET_START, { 0,0,0 });
        cpuInstanceBuffer.resize(MAX_INSTANCES, 0);

        globalInstanceBuffers.resize(frameCount);
        globalIndirectCommandBuffers.resize(frameCount);
        globalIndirectCommandDescriptorBuffers.resize(frameCount);
        globalDrawCountBuffers.resize(frameCount);
        globalModelAllocationBuffers.resize(frameCount);
        globalMeshAllocationBuffers.resize(frameCount);

        size_t traditionalBytes = MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
        size_t meshletBytes = (MAX_INDIRECT_COMMANDS - MESHLET_OFFSET_START) * sizeof(VkDrawMeshTasksIndirectCommandEXT);

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            globalInstanceBuffers[i] = Vk::BufferFactory::CreatePersistent(
                MAX_INSTANCES * sizeof(uint32_t),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );

            globalIndirectCommandBuffers[i] = Vk::BufferFactory::CreatePersistent(
                traditionalBytes + meshletBytes,
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            globalDrawCountBuffers[i] = Vk::BufferFactory::CreatePersistent(
                2 * sizeof(uint32_t),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            globalIndirectCommandDescriptorBuffers[i] = Vk::BufferFactory::CreatePersistent(
                MAX_INDIRECT_COMMANDS * sizeof(MeshDrawDescriptor),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );

            globalModelAllocationBuffers[i] = Vk::BufferFactory::CreatePersistent(
                ModelManager::MAX_MODELS * sizeof(ModelAllocationInfo),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );

            globalMeshAllocationBuffers[i] = Vk::BufferFactory::CreatePersistent(
                MAX_INDIRECT_COMMANDS * sizeof(MeshAllocationInfo),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );
        }
    }
}