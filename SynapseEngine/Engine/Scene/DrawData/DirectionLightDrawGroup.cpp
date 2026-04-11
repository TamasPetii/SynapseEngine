#include "DirectionLightDrawGroup.h"

namespace Syn
{
    DirectionLightDrawGroup::DirectionLightDrawGroup(uint32_t frameCount)
    {
        cmdTemplate.vertexCount = 3;
        cmdTemplate.instanceCount = 0;
        cmdTemplate.firstVertex = 0;
        cmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        indirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        indirectBuffer.UpdateCapacityAll(1);

        billboardSingleCmdBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        billboardSingleCmdBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            billboardSingleCmdBuffer.GetMapped(i)->Write(&billboardCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
        }
    }
}