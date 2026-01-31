#include "TimestampQueryPool.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk
{
    TimestampQueryPool::TimestampQueryPool(uint32_t queryCount)
        : QueryPool(VK_QUERY_TYPE_TIMESTAMP, queryCount)
    {
    }

    void TimestampQueryPool::WriteTimestamp(VkCommandBuffer cmd, VkPipelineStageFlagBits stage, uint32_t queryIndex) {
        vkCmdWriteTimestamp(cmd, stage, _handle, queryIndex);
    }
}