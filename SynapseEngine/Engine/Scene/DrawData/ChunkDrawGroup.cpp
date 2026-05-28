#include "ChunkDrawGroup.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn
{
    ChunkDrawGroup::ChunkDrawGroup(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);

        wireframeCmdTemplate.vertexCount = cube->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        wireframeCmdTemplate.instanceCount = 0;
        wireframeCmdTemplate.firstVertex = cube->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        wireframeCmdTemplate.firstInstance = 0;

        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |  VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        //Todo: Correct BufferStrategy

        chunkDataBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(ChunkDataGPU), storageUsage });
        chunkDataBuffer.UpdateCapacityAll(1);

        chunkVisibilityBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(uint32_t), storageUsage });
        chunkVisibilityBuffer.UpdateCapacityAll(1);

        chunkAabbSingleCmdBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        chunkAabbSingleCmdBuffer.UpdateCapacityAll(1);

        chunkIndirectDispatchBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectUsage });
        chunkIndirectDispatchBuffer.UpdateCapacityAll(1);

        sceneAabbBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(SceneAABB), storageUsage });
        sceneAabbBuffer.UpdateCapacityAll(1);

        mortonRadixSortTempBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, 1, storageUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 10000, 20000 });
        mortonRadixSortTempBuffer.UpdateCapacityAll(1);

        mortonIndirectDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectUsage });
        mortonIndirectDispatchBuffer.UpdateCapacityAll(1);

        mortonIndirectDrawBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        mortonIndirectDrawBuffer.UpdateCapacityAll(1);

        mortonChunkVisibleIndirectDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectUsage });
        mortonChunkVisibleIndirectDispatchBuffer.UpdateCapacityAll(1);

        mortonAabbSingleCmdBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        mortonAabbSingleCmdBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            chunkAabbSingleCmdBuffer.GetMapped(i)->Write(&wireframeCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            mortonAabbSingleCmdBuffer.GetMapped(i)->Write(&wireframeCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            chunkIndirectDispatchBuffer.GetMapped(i)->Write(&dispatchCmdTemplate, sizeof(VkDispatchIndirectCommand), 0);
        }
    }

    void ChunkDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        // Opcionális pipeline barrier szinkronizáció, ha a render graph igényli
    }
}