#include "DirectionLightShadowDrawGroup.h"

namespace Syn
{
    DirectionLightShadowDrawGroup::DirectionLightShadowDrawGroup(uint32_t frameCount)
    {
        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        paddedTraditionalCounts.AssignZero(16);
        paddedMeshletCounts.AssignZero(16);

        instances.AssignZero(1);

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        instanceBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(uint32_t), storageUsage, 16384 * SHADOW_MULTIPLIER, 32768 * SHADOW_MULTIPLIER });
        instanceBuffer.UpdateCapacityAll(1);

        indirectBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        computeCountBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        computeCountBuffer.UpdateCapacityAll(1);
    }

    void DirectionLightShadowDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {

    }
}