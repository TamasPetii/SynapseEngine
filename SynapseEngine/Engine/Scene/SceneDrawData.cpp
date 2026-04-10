#include "SceneDrawData.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn
{
    SceneDrawData::SceneDrawData(uint32_t frameCount)
    {
        InitModelBuffers(frameCount);
        InitDebugBuffers(frameCount);
        InitPointLightBuffers(frameCount);
		InitSpotLightBuffers(frameCount);
        InitDirectionLightBuffers(frameCount);
    }

    void SceneDrawData::InitModelBuffers(uint32_t frameCount)
    {
        paddedTraditionalCounts.resize(MAX_INDIRECT_COMMANDS * 16, 0);
        paddedMeshletCounts.resize(MAX_INDIRECT_COMMANDS * 16, 0);

        modelAllocations.resize(ModelManager::MAX_MODELS);
        meshAllocations.resize(MAX_INDIRECT_COMMANDS);
        drawDescriptors.resize(MAX_INDIRECT_COMMANDS);
        traditionalCommands.resize(MAX_INDIRECT_COMMANDS, { 0, 0, 0, 0 });
        meshletCommands.resize(MAX_INDIRECT_COMMANDS, { 0, 0, 0 });
        cpuInstanceBuffer.resize(MAX_INSTANCES, 0);

        mappedDrawCountBuffers.resize(frameCount);

        mappedInstanceBuffers.resize(frameCount);
        mappedIndirectCommandBuffers.resize(frameCount);
        mappedIndirectCommandDescriptorBuffers.resize(frameCount);
        mappedModelAllocationBuffers.resize(frameCount);
        mappedMeshAllocationBuffers.resize(frameCount);
        mappedMaterialIndexBuffers.resize(frameCount);

        gpuInstanceBuffers.resize(frameCount);
        gpuIndirectCommandBuffers.resize(frameCount);
        gpuIndirectCommandDescriptorBuffers.resize(frameCount);
        gpuModelAllocationBuffers.resize(frameCount);
        gpuMeshAllocationBuffers.resize(frameCount);
        gpuMaterialIndexBuffers.resize(frameCount);
        gpuModelComputeCountBuffers.resize(frameCount);

        const VkDeviceSize instanceBufferSize = MAX_INSTANCES * sizeof(uint32_t);
        const VkDeviceSize indirectCommandBufferSize = (MAX_INDIRECT_COMMANDS * sizeof(VkDrawIndirectCommand) + MAX_INDIRECT_COMMANDS * sizeof(VkDrawMeshTasksIndirectCommandEXT)) / 2;
        const VkDeviceSize drawCountBufferSize = (MaterialRenderType::Count * 2) * sizeof(uint32_t);
        const VkDeviceSize descriptorBufferSize = MAX_INDIRECT_COMMANDS * sizeof(MeshDrawDescriptor);
        const VkDeviceSize modelAllocationBufferSize = ModelManager::MAX_MODELS * sizeof(ModelAllocationInfo);
        const VkDeviceSize meshAllocationBufferSize = MAX_INDIRECT_COMMANDS * sizeof(MeshAllocationInfo);
        const VkDeviceSize computeCountBufferSize = sizeof(VkDispatchIndirectCommand);
        const VkDeviceSize materialIndexBufferSize = sizeof(int32_t);

        const VkBufferUsageFlags mappedStorageUsage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        const VkBufferUsageFlags mappedIndirectUsage =
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        const VkBufferUsageFlags gpuStorageUsage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        const VkBufferUsageFlags gpuIndirectUsage =
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            mappedDrawCountBuffers[i] = Vk::BufferFactory::CreatePersistent(drawCountBufferSize, mappedIndirectUsage);

            mappedInstanceBuffers[i] = Vk::BufferFactory::CreatePersistent(instanceBufferSize, mappedStorageUsage);
            mappedIndirectCommandBuffers[i] = Vk::BufferFactory::CreatePersistent(indirectCommandBufferSize, mappedIndirectUsage);
            mappedIndirectCommandDescriptorBuffers[i] = Vk::BufferFactory::CreatePersistent(descriptorBufferSize, mappedStorageUsage);
            mappedModelAllocationBuffers[i] = Vk::BufferFactory::CreatePersistent(modelAllocationBufferSize, mappedStorageUsage);
            mappedMeshAllocationBuffers[i] = Vk::BufferFactory::CreatePersistent(meshAllocationBufferSize, mappedStorageUsage);
            mappedMaterialIndexBuffers[i] = Vk::BufferFactory::CreatePersistent(materialIndexBufferSize, mappedStorageUsage);

            gpuInstanceBuffers[i] = Vk::BufferFactory::CreateGpu(instanceBufferSize, gpuStorageUsage);
            gpuIndirectCommandBuffers[i] = Vk::BufferFactory::CreateGpu(indirectCommandBufferSize, gpuIndirectUsage);
            gpuIndirectCommandDescriptorBuffers[i] = Vk::BufferFactory::CreateGpu(descriptorBufferSize, gpuStorageUsage);
            gpuModelAllocationBuffers[i] = Vk::BufferFactory::CreateGpu(modelAllocationBufferSize, gpuStorageUsage);
            gpuMeshAllocationBuffers[i] = Vk::BufferFactory::CreateGpu(meshAllocationBufferSize, gpuStorageUsage);
            gpuModelComputeCountBuffers[i] = Vk::BufferFactory::CreateGpu(computeCountBufferSize, gpuIndirectUsage);
            gpuMaterialIndexBuffers[i] = Vk::BufferFactory::CreateGpu(materialIndexBufferSize, gpuStorageUsage);
        }
    }

    void SceneDrawData::InitDebugBuffers(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);

        aabbIndirectCommands.resize(MAX_INDIRECT_COMMANDS);
        sphereIndirectCommands.resize(MAX_INDIRECT_COMMANDS);

        aabbIndirectCommandBuffers.resize(frameCount);
        sphereIndirectCommandBuffers.resize(frameCount);

        debugInstanceBuffers.resize(frameCount);
        debugAabbIndirectBuffers.resize(frameCount);
        debugSphereIndirectBuffers.resize(frameCount);

        debugAabbCmdTemplate.vertexCount = cube->baseDrawCommands[0].traditionalCmd.vertexCount;
        debugAabbCmdTemplate.instanceCount = 0;
        debugAabbCmdTemplate.firstVertex = cube->baseDrawCommands[0].traditionalCmd.firstVertex;
        debugAabbCmdTemplate.firstInstance = 0;

        debugSphereCmdTemplate.vertexCount = sphere->baseDrawCommands[0].traditionalCmd.vertexCount;
        debugSphereCmdTemplate.instanceCount = 0;
        debugSphereCmdTemplate.firstVertex = sphere->baseDrawCommands[0].traditionalCmd.firstVertex;
        debugSphereCmdTemplate.firstInstance = 0;

        std::vector<VkDrawIndirectCommand> aabbCommands(
            MAX_INDIRECT_COMMANDS,
            debugAabbCmdTemplate
        );

        std::vector<VkDrawIndirectCommand> sphereCommands(
            MAX_INDIRECT_COMMANDS,
            debugSphereCmdTemplate
        );

        const VkDeviceSize indirectCommandBufferSize =
            MAX_INDIRECT_COMMANDS * sizeof(VkDrawIndirectCommand);

        const VkDeviceSize singleIndirectCommandSize =
            sizeof(VkDrawIndirectCommand);

        const VkDeviceSize debugInstanceBufferSize =
            sizeof(uint32_t);

        const VkBufferUsageFlags indirectStorageTransferUsage =
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        const VkBufferUsageFlags storageTransferUsage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            aabbIndirectCommandBuffers[i] =
                Vk::BufferFactory::CreatePersistent(
                    indirectCommandBufferSize,
                    indirectStorageTransferUsage
                );

            sphereIndirectCommandBuffers[i] =
                Vk::BufferFactory::CreatePersistent(
                    indirectCommandBufferSize,
                    indirectStorageTransferUsage
                );

            debugAabbIndirectBuffers[i] =
                Vk::BufferFactory::CreatePersistent(
                    singleIndirectCommandSize,
                    indirectStorageTransferUsage
                );

            debugSphereIndirectBuffers[i] =
                Vk::BufferFactory::CreatePersistent(
                    singleIndirectCommandSize,
                    indirectStorageTransferUsage
                );

            debugInstanceBuffers[i] =
                Vk::BufferFactory::CreatePersistent(
                    debugInstanceBufferSize,
                    storageTransferUsage
                );

            aabbIndirectCommandBuffers[i]->Write(
                aabbCommands.data(),
                aabbCommands.size() * sizeof(VkDrawIndirectCommand)
            );

            sphereIndirectCommandBuffers[i]->Write(
                sphereCommands.data(),
                sphereCommands.size() * sizeof(VkDrawIndirectCommand)
            );
        }
    }

    void SceneDrawData::InitPointLightBuffers(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);

        pointLightIndirectCommandBuffers.resize(frameCount);
        debugPointLightSphereCmdBuffers.resize(frameCount);
        debugPointLightAabbCmdBuffers.resize(frameCount);
        editorPointLightBillboardCmdBuffers.resize(frameCount);

        VkDrawIndirectCommand sphereCmdTemplate{};
        sphereCmdTemplate.vertexCount = sphere->baseDrawCommands[0].traditionalCmd.vertexCount;
        sphereCmdTemplate.instanceCount = 0;
        sphereCmdTemplate.firstVertex = sphere->baseDrawCommands[0].traditionalCmd.firstVertex;
        sphereCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand aabbCmdTemplate{};
        aabbCmdTemplate.vertexCount = cube->baseDrawCommands[0].traditionalCmd.vertexCount;
        aabbCmdTemplate.instanceCount = 0;
        aabbCmdTemplate.firstVertex = cube->baseDrawCommands[0].traditionalCmd.firstVertex;
        aabbCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

        pointLightCmdTemplate = aabbCmdTemplate;

        const VkDeviceSize cmdSize = sizeof(VkDrawIndirectCommand);

        const VkBufferUsageFlags usage =
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            pointLightIndirectCommandBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);

            debugPointLightSphereCmdBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);
            debugPointLightSphereCmdBuffers[i]->Write(&sphereCmdTemplate, cmdSize, 0);

            debugPointLightAabbCmdBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);
            debugPointLightAabbCmdBuffers[i]->Write(&aabbCmdTemplate, cmdSize, 0);

            editorPointLightBillboardCmdBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);
            editorPointLightBillboardCmdBuffers[i]->Write(&billboardCmdTemplate, cmdSize, 0);
        }
    }

    void SceneDrawData::InitSpotLightBuffers(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);
        auto pyramid = modelManager->GetResource(MeshSourceNames::Pyramid);

        spotLightIndirectCommandBuffers.resize(frameCount);
        debugSpotLightSphereCmdBuffers.resize(frameCount);
        debugSpotLightAabbCmdBuffers.resize(frameCount);
        editorSpotLightBillboardCmdBuffers.resize(frameCount);

        VkDrawIndirectCommand sphereCmdTemplate{};
        sphereCmdTemplate.vertexCount = sphere->baseDrawCommands[0].traditionalCmd.vertexCount;
        sphereCmdTemplate.instanceCount = 0;
        sphereCmdTemplate.firstVertex = sphere->baseDrawCommands[0].traditionalCmd.firstVertex;
        sphereCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand aabbCmdTemplate{};
        aabbCmdTemplate.vertexCount = cube->baseDrawCommands[0].traditionalCmd.vertexCount;
        aabbCmdTemplate.instanceCount = 0;
        aabbCmdTemplate.firstVertex = cube->baseDrawCommands[0].traditionalCmd.firstVertex;
        aabbCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand pyramidCmdTemplate{};
        pyramidCmdTemplate.vertexCount = pyramid->baseDrawCommands[0].traditionalCmd.vertexCount;
        pyramidCmdTemplate.instanceCount = 0;
        pyramidCmdTemplate.firstVertex = pyramid->baseDrawCommands[0].traditionalCmd.firstVertex;
        pyramidCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

		spotLightCmdTemplate = pyramidCmdTemplate;

        const VkDeviceSize cmdSize = sizeof(VkDrawIndirectCommand);

        const VkBufferUsageFlags usage =
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            spotLightIndirectCommandBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);

            debugSpotLightSphereCmdBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);
            debugSpotLightSphereCmdBuffers[i]->Write(&sphereCmdTemplate, cmdSize, 0);

            debugSpotLightAabbCmdBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);
            debugSpotLightAabbCmdBuffers[i]->Write(&aabbCmdTemplate, cmdSize, 0);

            editorSpotLightBillboardCmdBuffers[i] = Vk::BufferFactory::CreatePersistent(cmdSize, usage);
            editorSpotLightBillboardCmdBuffers[i]->Write(&billboardCmdTemplate, cmdSize, 0);
        }
    }

    void SceneDrawData::InitDirectionLightBuffers(uint32_t frameCount)
    {
        directionLightIndirectCommandBuffers.resize(frameCount);
        editorDirectionLightBillboardCmdBuffers.resize(frameCount);

        directionLightCmdTemplate.vertexCount = 3;
        directionLightCmdTemplate.instanceCount = 0;
        directionLightCmdTemplate.firstVertex = 0;
        directionLightCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

        const VkDeviceSize indirectCommandSize = sizeof(VkDrawIndirectCommand);

        const VkBufferUsageFlags indirectStorageTransferUsage =
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            directionLightIndirectCommandBuffers[i] =
                Vk::BufferFactory::CreatePersistent(
                    indirectCommandSize,
                    indirectStorageTransferUsage
                );

            editorDirectionLightBillboardCmdBuffers[i] =
                Vk::BufferFactory::CreatePersistent(indirectCommandSize, indirectStorageTransferUsage);
            editorDirectionLightBillboardCmdBuffers[i]->Write(&billboardCmdTemplate, indirectCommandSize, 0);
        }
    }

    void SceneDrawData::RecordGpuSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        uint32_t currentSync = syncFramesRemaining.load(std::memory_order_relaxed);
        if (currentSync == 0) return;

        size_t modelAllocSize = modelAllocations.size() * sizeof(ModelAllocationInfo);
        if (modelAllocSize > 0) {
            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = mappedModelAllocationBuffers[frameIndex]->Handle();
            copyInfo.dstBuffer = gpuModelAllocationBuffers[frameIndex]->Handle();
            copyInfo.size = modelAllocSize;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = 0;
            Vk::BufferUtils::CopyBuffer(cmd, copyInfo);
        }

        size_t meshAllocSize = activeDescriptorCount * sizeof(MeshAllocationInfo);
        if (meshAllocSize > 0) {
            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = mappedMeshAllocationBuffers[frameIndex]->Handle();
            copyInfo.dstBuffer = gpuMeshAllocationBuffers[frameIndex]->Handle();
            copyInfo.size = meshAllocSize;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = 0;
            Vk::BufferUtils::CopyBuffer(cmd, copyInfo);
        }

        uint32_t totalDescriptors = activeTraditionalCount + activeMeshletCount;
        size_t descSize = totalDescriptors * sizeof(MeshDrawDescriptor);
        if (descSize > 0) {
            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = mappedIndirectCommandDescriptorBuffers[frameIndex]->Handle();
            copyInfo.dstBuffer = gpuIndirectCommandDescriptorBuffers[frameIndex]->Handle();
            copyInfo.size = descSize;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = 0;
            Vk::BufferUtils::CopyBuffer(cmd, copyInfo);
        }

        if (requiredMaterialBufferSize > 0 && mappedMaterialIndexBuffers[frameIndex] && gpuMaterialIndexBuffers[frameIndex]) {

            size_t mappedSize = mappedMaterialIndexBuffers[frameIndex]->GetSize();
            size_t gpuSize = gpuMaterialIndexBuffers[frameIndex]->GetSize();
            size_t copySize = std::min({ (size_t)requiredMaterialBufferSize, mappedSize, gpuSize });

            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = mappedMaterialIndexBuffers[frameIndex]->Handle();
            copyInfo.dstBuffer = gpuMaterialIndexBuffers[frameIndex]->Handle();
            copyInfo.size = copySize;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = 0;
            Vk::BufferUtils::CopyBuffer(cmd, copyInfo);
        }

        size_t tradSize = activeTraditionalCount * sizeof(VkDrawIndirectCommand);
        size_t meshletSize = activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
        size_t totalCommandSize = tradSize + meshletSize;

        if (totalCommandSize > 0) {
            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = mappedIndirectCommandBuffers[frameIndex]->Handle();
            copyInfo.dstBuffer = gpuIndirectCommandBuffers[frameIndex]->Handle();
            copyInfo.size = totalCommandSize;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = 0;
            Vk::BufferUtils::CopyBuffer(cmd, copyInfo);
        }

        Vk::GlobalBarrierInfo barrierInfo{};
        barrierInfo.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrierInfo.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrierInfo.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        barrierInfo.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
        Vk::BufferUtils::InsertGlobalBarrier(cmd, barrierInfo);

        uint32_t expected = currentSync;
        while (expected > 0 && !syncFramesRemaining.compare_exchange_weak(expected, expected - 1, std::memory_order_relaxed)) {}
    }
}