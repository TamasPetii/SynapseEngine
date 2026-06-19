#include "ModelDrawGroup.h"
#include "Engine/Mesh/ModelManager.h"

namespace Syn
{
    ModelDrawGroup::ModelDrawGroup(uint32_t frameCount)
    {
        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        paddedTraditionalCounts.AssignZero(16);
        paddedMeshletCounts.AssignZero(16);

        instances.AssignZero(1);
        traditionalCmds.AssignZero(1);
        meshletCmds.AssignZero(1);
        descriptors.AssignZero(1);
        meshAllocations.AssignZero(1);
        modelAllocations.AssignZero(1);

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        instanceBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(uint32_t), storageUsage, 16384, 32768 });
        instanceBuffer.UpdateCapacityAll(1);

		//VkDrawIndirectCommand + VkDrawMeshTasksIndirectCommandEXT
        indirectBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        descriptorBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(MeshDrawDescriptor), storageUsage, 1024, 2048 });
        descriptorBuffer.UpdateCapacityAll(1);

        modelAllocBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(ModelAllocationInfo), storageUsage, 1024, 2048 });
        modelAllocBuffer.UpdateCapacityAll(1);

        meshAllocBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(MeshAllocationInfo), storageUsage, 1024, 2048 });
        meshAllocBuffer.UpdateCapacityAll(1);

        materialIndexBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(int32_t), storageUsage, 1024, 2048 });
        materialIndexBuffer.UpdateCapacityAll(1);

        drawCountBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(uint32_t), indirectStorageUsage, 1, 1 });
        drawCountBuffer.UpdateCapacityAll(MaterialRenderType::Count * 2);

        computeCountBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        computeCountBuffer.UpdateCapacityAll(1);
    }

    void ModelDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        instanceBuffer.RecordSync(cmd, frameIndex);
        indirectBuffer.RecordSync(cmd, frameIndex);
        descriptorBuffer.RecordSync(cmd, frameIndex);
        modelAllocBuffer.RecordSync(cmd, frameIndex);
        meshAllocBuffer.RecordSync(cmd, frameIndex);
        materialIndexBuffer.RecordSync(cmd, frameIndex);
    }
}