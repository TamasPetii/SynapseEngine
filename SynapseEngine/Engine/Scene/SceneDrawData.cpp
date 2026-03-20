#include "SceneDrawData.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn
{
    SceneDrawData::SceneDrawData(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();

        modelAllocations.resize(ModelManager::MAX_MODELS);
        meshAllocations.resize(MAX_INDIRECT_COMMANDS);
        drawDescriptors.resize(MAX_INDIRECT_COMMANDS);
        traditionalCommands.resize(MESHLET_OFFSET_START, { 0,0,0,0 });
        meshletCommands.resize(MAX_INDIRECT_COMMANDS - MESHLET_OFFSET_START, { 0,0,0 });
        cpuInstanceBuffer.resize(MAX_INSTANCES, 0);
        aabbIndirectCommands.resize(MAX_INDIRECT_COMMANDS);
        sphereIndirectCommands.resize(MAX_INDIRECT_COMMANDS);

        globalInstanceBuffers.resize(frameCount);
        globalIndirectCommandBuffers.resize(frameCount);
        globalIndirectCommandDescriptorBuffers.resize(frameCount);
        globalDrawCountBuffers.resize(frameCount);
        globalModelAllocationBuffers.resize(frameCount);
        globalMeshAllocationBuffers.resize(frameCount);
        globalModelComputeCountBuffer.resize(frameCount);
        globalMaterialIndexBuffers.resize(frameCount);

        aabbIndirectCommandBuffers.resize(frameCount);
        sphereIndirectCommandBuffers.resize(frameCount);

        debugInstanceBuffers.resize(frameCount);
        debugAabbIndirectBuffers.resize(frameCount);
        debugSphereIndirectBuffers.resize(frameCount);

        size_t traditionalBytes = MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
        size_t meshletBytes = (MAX_INDIRECT_COMMANDS - MESHLET_OFFSET_START) * sizeof(VkDrawMeshTasksIndirectCommandEXT);

        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        VkDrawIndirectCommand aabbCmd{};
        aabbCmd.vertexCount = cube->baseDrawCommands[0].traditionalCmd.vertexCount;
        aabbCmd.instanceCount = 0;
        aabbCmd.firstVertex = cube->baseDrawCommands[0].traditionalCmd.firstVertex;;
        aabbCmd.firstInstance = 0;

        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);
        VkDrawIndirectCommand sphereCmd{};
        sphereCmd.vertexCount = sphere->baseDrawCommands[0].traditionalCmd.vertexCount;
        sphereCmd.instanceCount = 0;
        sphereCmd.firstVertex = sphere->baseDrawCommands[0].traditionalCmd.firstVertex;
        sphereCmd.firstInstance = 0;

        std::vector<VkDrawIndirectCommand> aabbCommands(MAX_INDIRECT_COMMANDS, aabbCmd);
        std::vector<VkDrawIndirectCommand> sphereCommands(MAX_INDIRECT_COMMANDS, sphereCmd);

        debugAabbCmdTemplate = aabbCmd;
        debugSphereCmdTemplate = sphereCmd;

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

            globalModelComputeCountBuffer[i] = Vk::BufferFactory::CreatePersistent(
                1 * sizeof(VkDispatchIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );

            globalMaterialIndexBuffers[i] = Vk::BufferFactory::CreatePersistent(
                sizeof(int32_t),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );

            aabbIndirectCommandBuffers[i] = Vk::BufferFactory::CreatePersistent(
                MAX_INDIRECT_COMMANDS * sizeof(VkDrawIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            sphereIndirectCommandBuffers[i] = Vk::BufferFactory::CreatePersistent(
                MAX_INDIRECT_COMMANDS * sizeof(VkDrawIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            debugAabbIndirectBuffers[i] = Vk::BufferFactory::CreatePersistent(
                sizeof(VkDrawIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            debugSphereIndirectBuffers[i] = Vk::BufferFactory::CreatePersistent(
                sizeof(VkDrawIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            debugInstanceBuffers[i] = Vk::BufferFactory::CreatePersistent(
                sizeof(uint32_t),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );

            aabbIndirectCommandBuffers[i]->Write(aabbCommands.data(), aabbCommands.size() * sizeof(VkDrawIndirectCommand));
            sphereIndirectCommandBuffers[i]->Write(sphereCommands.data(), sphereCommands.size() * sizeof(VkDrawIndirectCommand));
        }
    }
}