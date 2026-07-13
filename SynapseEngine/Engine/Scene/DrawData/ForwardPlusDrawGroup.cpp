#include "ForwardPlusDrawGroup.h"

namespace Syn
{
    ForwardPlusDrawGroup::ForwardPlusDrawGroup(uint32_t frameCount)
    {
        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        tileGridBuffer.Initialize({ "ForwardPlusDrawGroup_TileGridBuffer", BufferStrategy::GpuOnly, frameCount, 16, storageUsage, 3000, 6000 });
		tileGridBuffer.UpdateCapacityAll(1); 

        clusterListBuffer.Initialize({ "ForwardPlusDrawGroup_ClusterListBuffer", BufferStrategy::GpuOnly, frameCount, 32, storageUsage, 48000, 96000 });
        clusterListBuffer.UpdateCapacityAll(1);

        clusterCountBuffer.Initialize({ "ForwardPlusDrawGroup_ClusterCountBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectUsage, 1, 1});
        clusterCountBuffer.UpdateCapacityAll(1);

        dispatchArgsBuffer.Initialize({ "ForwardPlusDrawGroup_DispatchArgsBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(ForwardPlusDispatchArgs), indirectUsage, 1, 1 });
        dispatchArgsBuffer.UpdateCapacityAll(1);

        pointLightIndexBuffer.Initialize({ "ForwardPlusDrawGroup_PointLightIndexBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 10000, 20000 });
        pointLightIndexBuffer.UpdateCapacityAll(1);
        
        spotLightIndexBuffer.Initialize({ "ForwardPlusDrawGroup_SpotLightIndexBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 10000, 20000 });
        spotLightIndexBuffer.UpdateCapacityAll(1);
    }

    void ForwardPlusDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {

    }

    void ForwardPlusDrawGroup::CheckResize(uint32_t tileSize, uint32_t width, uint32_t height, uint32_t frameIndex)
    {
        ComputeGroupSize::CalculateDispatchCount(width, tileSize);

        uint32_t gridX = ComputeGroupSize::CalculateDispatchCount(width, tileSize);
        uint32_t gridY = ComputeGroupSize::CalculateDispatchCount(height, tileSize);
        uint32_t gridZ = 16;

        maxClusters = gridX * gridY * gridZ;

        tileGridBuffer.UpdateCapacity(frameIndex, gridX * gridY);
        clusterListBuffer.UpdateCapacity(frameIndex, maxClusters);

        uint32_t expectedAverageLightsPerCluster = 1024;
        uint32_t totalLightIndices = maxClusters * expectedAverageLightsPerCluster;

        pointLightIndexBuffer.UpdateCapacity(frameIndex, totalLightIndices);
        spotLightIndexBuffer.UpdateCapacity(frameIndex, totalLightIndices);
    }
}