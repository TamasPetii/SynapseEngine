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

        modelAabbCmdTemplate.vertexCount = cube->baseDrawCommands[0].traditionalCmd.vertexCount;
        modelAabbCmdTemplate.instanceCount = 0;
        modelAabbCmdTemplate.firstVertex = cube->baseDrawCommands[0].traditionalCmd.firstVertex;
        modelAabbCmdTemplate.firstInstance = 0;

        modelSphereCmdTemplate.vertexCount = sphere->baseDrawCommands[0].traditionalCmd.vertexCount;
        modelSphereCmdTemplate.instanceCount = 0;
        modelSphereCmdTemplate.firstVertex = sphere->baseDrawCommands[0].traditionalCmd.firstVertex;
        modelSphereCmdTemplate.firstInstance = 0;

        modelAabbCmds.data.assign(MAX_INDIRECT_COMMANDS, modelAabbCmdTemplate);
        modelSphereCmds.data.assign(MAX_INDIRECT_COMMANDS, modelSphereCmdTemplate);

        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        modelAabbIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        modelAabbIndirectBuffer.UpdateCapacityAll(MAX_INDIRECT_COMMANDS);

        modelSphereIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        modelSphereIndirectBuffer.UpdateCapacityAll(MAX_INDIRECT_COMMANDS);

        meshletAabbIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        meshletAabbIndirectBuffer.UpdateCapacityAll(1);

        meshletSphereIndirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        meshletSphereIndirectBuffer.UpdateCapacityAll(1);

        instanceBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(uint32_t), storageUsage });
        instanceBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            modelAabbIndirectBuffer.GetMapped(i)->Write(modelAabbCmds.Data(), modelAabbCmds.GetSizeBytes(), 0);
            modelSphereIndirectBuffer.GetMapped(i)->Write(modelSphereCmds.Data(), modelSphereCmds.GetSizeBytes(), 0);
        }
    }
}