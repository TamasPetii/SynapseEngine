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
		VkQueryPipelineStatisticFlags GetFlags() const { return _flags; }
    protected:
        uint32_t GetStride() const override;
    private:
        VkQueryPipelineStatisticFlags _flags;
    };
}