#include "PipelineStatisticsQueryPool.h"

namespace Syn::Vk
{
    PipelineStatisticsQueryPool::PipelineStatisticsQueryPool(uint32_t queryCount, VkQueryPipelineStatisticFlags flags)
        : QueryPool(VK_QUERY_TYPE_PIPELINE_STATISTICS, queryCount, flags)
    {}

    void PipelineStatisticsQueryPool::BeginQuery(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdBeginQuery(cmd, _handle, queryIndex, 0);
    }

    void PipelineStatisticsQueryPool::EndQuery(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdEndQuery(cmd, _handle, queryIndex);
    }
}