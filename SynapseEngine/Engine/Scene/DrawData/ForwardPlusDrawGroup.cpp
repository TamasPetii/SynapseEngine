#include "ForwardPlusDrawGroup.h"

namespace Syn
{
    ForwardPlusDrawGroup::ForwardPlusDrawGroup(uint32_t frameCount)
    {
        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        tileGridBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, 16, storageUsage, 3000, 6000 });
        
        clusterListBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, 32, storageUsage, 48000, 96000 });

        clusterCountBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t) * 4, indirectUsage, 1, 1 });
        clusterCountBuffer.UpdateCapacityAll(1);

        lightCounterBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t) * 2, storageUsage, 1, 1 });
        lightCounterBuffer.UpdateCapacityAll(1);

        pointLightIndexBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 100000, 200000 });
        spotLightIndexBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 100000, 200000 });
    }

    void ForwardPlusDrawGroup::RecordSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        //todo update: maxClusters too
    }
}