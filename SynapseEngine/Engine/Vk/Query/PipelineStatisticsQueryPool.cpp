#include "PipelineStatisticsQueryPool.h"
#include <bit>

namespace Syn::Vk
{
    PipelineStatisticsQueryPool::PipelineStatisticsQueryPool(uint32_t queryCount, VkQueryPipelineStatisticFlags flags)
        : QueryPool(VK_QUERY_TYPE_PIPELINE_STATISTICS, queryCount, flags), _flags(flags)
    {}

    void PipelineStatisticsQueryPool::BeginQuery(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdBeginQuery(cmd, _handle, queryIndex, 0);
    }

    void PipelineStatisticsQueryPool::EndQuery(VkCommandBuffer cmd, uint32_t queryIndex) {
        vkCmdEndQuery(cmd, _handle, queryIndex);
    }

    uint32_t PipelineStatisticsQueryPool::GetStride() const {
        return std::popcount(static_cast<uint32_t>(_flags)) * sizeof(uint64_t);
    }
}