#include "DebugDrawGroup.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Material/MaterialRenderType.h"
#include "Engine/Physics/PhysicsTypes.h"

namespace Syn
{
    DebugDrawGroup::DebugDrawGroup(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::Get<ModelManager>();
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

        modelAabbCmds.data.assign(1, modelAabbCmdTemplate);
        modelSphereCmds.data.assign(1, modelSphereCmdTemplate);

        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        modelAabbIndirectBuffer.Initialize({ "DebugDrawGroup_ModelAabbIndirectBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand) * MaterialRenderType::MaterialRenderTypeCount * 2, indirectUsage, 1024, 2048 });
        modelAabbIndirectBuffer.UpdateCapacityAll(1);

        modelSphereIndirectBuffer.Initialize({ "DebugDrawGroup_ModelSphereIndirectBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand) * MaterialRenderType::MaterialRenderTypeCount * 2, indirectUsage, 1024, 2048 });
        modelSphereIndirectBuffer.UpdateCapacityAll(1);

        joltDebugVertexBuffer.Initialize({ "DebugDrawGroup_JoltDebugVertexBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(PhysicsDebugVertex), storageUsage, 100000, 200000 });
        joltDebugVertexBuffer.UpdateCapacityAll(1);

        joltDebugIndexBuffer.Initialize({ "DebugDrawGroup_JoltIndex", BufferStrategy::MappedOnly, frameCount, sizeof(uint32_t), storageUsage, 200000, 400000 });
        joltDebugIndexBuffer.UpdateCapacityAll(1);

        joltDebugInstanceBuffer.Initialize({ "DebugDrawGroup_JoltInstance", BufferStrategy::MappedOnly, frameCount, sizeof(PhysicsDebugInstance), storageUsage, 10000, 20000 });
        joltDebugInstanceBuffer.UpdateCapacityAll(1);

        joltDebugIndirectBuffer.Initialize({ "DebugDrawGroup_JoltIndirect", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage, 10000, 20000 });
        joltDebugIndirectBuffer.UpdateCapacityAll(1);
    }

    void DebugDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {

    }
}