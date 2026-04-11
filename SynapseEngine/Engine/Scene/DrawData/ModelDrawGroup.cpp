#include "ModelDrawGroup.h"
#include "Engine/Mesh/ModelManager.h"

namespace Syn
{
    ModelDrawGroup::ModelDrawGroup(uint32_t frameCount)
    {
        paddedTraditionalCounts.AssignZero(MAX_INDIRECT_COMMANDS * 16);
        paddedMeshletCounts.AssignZero(MAX_INDIRECT_COMMANDS * 16);

        instances.AssignZero(MAX_INSTANCES);
        traditionalCmds.AssignZero(MAX_INDIRECT_COMMANDS);
        meshletCmds.AssignZero(MAX_INDIRECT_COMMANDS);
        descriptors.AssignZero(MAX_INDIRECT_COMMANDS);
        meshAllocations.AssignZero(MAX_INDIRECT_COMMANDS);
        modelAllocations.AssignZero(ModelManager::MAX_MODELS);

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        instanceBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(uint32_t), storageUsage });
        instanceBuffer.UpdateCapacityAll(MAX_INSTANCES);

        indirectBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, 1, indirectStorageUsage });
        indirectBuffer.UpdateCapacityAll((MAX_INDIRECT_COMMANDS * sizeof(VkDrawIndirectCommand) + MAX_INDIRECT_COMMANDS * sizeof(VkDrawMeshTasksIndirectCommandEXT)));

        descriptorBuffer.Initialize({ BufferStrategy::Hybrid_Static, frameCount, sizeof(MeshDrawDescriptor), storageUsage });
        descriptorBuffer.UpdateCapacityAll(MAX_INDIRECT_COMMANDS);

        modelAllocBuffer.Initialize({ BufferStrategy::Hybrid_Static, frameCount, sizeof(ModelAllocationInfo), storageUsage });
        modelAllocBuffer.UpdateCapacityAll(ModelManager::MAX_MODELS);

        meshAllocBuffer.Initialize({ BufferStrategy::Hybrid_Static, frameCount, sizeof(MeshAllocationInfo), storageUsage });
        meshAllocBuffer.UpdateCapacityAll(MAX_INDIRECT_COMMANDS);

        materialIndexBuffer.Initialize({ BufferStrategy::Hybrid_Static, frameCount, sizeof(int32_t), storageUsage });
        materialIndexBuffer.UpdateCapacityAll(1);

        drawCountBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(uint32_t), indirectStorageUsage });
        drawCountBuffer.UpdateCapacityAll(MaterialRenderType::Count * 2);

        computeCountBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage });
        computeCountBuffer.UpdateCapacityAll(1);
    }

    void ModelDrawGroup::RecordSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        modelAllocBuffer.RecordSync(cmd, frameIndex, modelAllocations.Size());
        meshAllocBuffer.RecordSync(cmd, frameIndex, activeDescriptorCount);
        descriptorBuffer.RecordSync(cmd, frameIndex, activeTraditionalCount + activeMeshletCount);

        if (requiredMaterialBufferSize > 0) {
            materialIndexBuffer.RecordSync(cmd, frameIndex, requiredMaterialBufferSize / sizeof(int32_t));
        }

        size_t tradSize = activeTraditionalCount * sizeof(VkDrawIndirectCommand);
        size_t meshletSize = activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
        size_t totalCommandSize = tradSize + meshletSize;

        if (totalCommandSize > 0) {
            indirectBuffer.RecordSync(cmd, frameIndex, totalCommandSize);
        }
    }
}