#pragma once
#include "../VkCommon.h"
#include "QueryPool.h"

namespace Syn::Vk
{
    class SYN_API PipelineStatisticsQueryPool : public QueryPool {
    public:
        PipelineStatisticsQueryPool(uint32_t queryCount, VkQueryPipelineStatisticFlags flags);

        void BeginQuery(VkCommandBuffer cmd, uint32_t queryIndex);
        void EndQuery(VkCommandBuffer cmd, uint32_t queryIndex);
    };
}