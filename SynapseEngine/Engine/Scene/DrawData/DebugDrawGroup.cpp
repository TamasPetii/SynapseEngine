#include "DebugDrawGroup.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn
{
    DebugDrawGroup::DebugDrawGroup(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);
		auto capsule = modelManager->GetResource(MeshSourceNames::Capsule);

        modelAabbCmdTemplate.vertexCount = cube->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        modelAabbCmdTemplate.instanceCount = 0;
        modelAabbCmdTemplate.firstVertex = cube->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        modelAabbCmdTemplate.firstInstance = 0;

        modelSphereCmdTemplate.vertexCount = sphere->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        modelSphereCmdTemplate.instanceCount = 0;
        modelSphereCmdTemplate.firstVertex = sphere->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        modelSphereCmdTemplate.firstInstance = 0;

        boxColliderCmdTemplate.vertexCount = cube->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        boxColliderCmdTemplate.instanceCount = 0;
        boxColliderCmdTemplate.firstVertex = cube->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        boxColliderCmdTemplate.firstInstance = 0;

        sphereColliderCmdTemplate.vertexCount = sphere->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        sphereColliderCmdTemplate.instanceCount = 0;
        sphereColliderCmdTemplate.firstVertex = sphere->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        sphereColliderCmdTemplate.firstInstance = 0;

        capsuleColliderCmdTemplate.vertexCount = capsule->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        capsuleColliderCmdTemplate.instanceCount = 0;
        capsuleColliderCmdTemplate.firstVertex = capsule->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        capsuleColliderCmdTemplate.firstInstance = 0;

        modelAabbCmds.data.assign(1, modelAabbCmdTemplate);
        modelSphereCmds.data.assign(1, modelSphereCmdTemplate);

        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        modelAabbIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectUsage, 1024, 2048 });
        modelAabbIndirectBuffer.UpdateCapacityAll(1);

        modelSphereIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectUsage, 1024, 2048 });
        modelSphereIndirectBuffer.UpdateCapacityAll(1);

        boxColliderIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        boxColliderIndirectBuffer.UpdateCapacityAll(1);

        sphereColliderIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        sphereColliderIndirectBuffer.UpdateCapacityAll(1);

        capsuleColliderIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        capsuleColliderIndirectBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            boxColliderIndirectBuffer.GetMapped(i)->Write(&boxColliderCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            sphereColliderIndirectBuffer.GetMapped(i)->Write(&sphereColliderCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            capsuleColliderIndirectBuffer.GetMapped(i)->Write(&capsuleColliderCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
        }

        //Todo: Meshlet visibility buffer?
    }

    void DebugDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {

    }
}