#include "OcclusionQueryPool.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk
{
    OcclusionQueryPool::OcclusionQueryPool(uint32_t queryCount)
        : QueryPool(VK_QUERY_TYPE_OCCLUSION, queryCount)
    {
    }

    void OcclusionQueryPool::BeginQuery(VkCommandBuffer cmd, uint32_t queryIndex, VkQueryControlFlags flags) {
        vkCmdBeginQuery(cmd, _handle, queryIndex, flags);
    }

    void OcclusionQueryPool::EndQuery(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdEndQuery(cmd, _handle, queryIndex);
    }
}