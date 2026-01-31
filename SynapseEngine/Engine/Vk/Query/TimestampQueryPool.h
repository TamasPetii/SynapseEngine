#pragma once
#include "../VkCommon.h"
#include "QueryPool.h"

namespace Syn::Vk
{
    class SYN_API TimestampQueryPool : public QueryPool {
    public:
        TimestampQueryPool(uint32_t queryCount);
        void WriteTimestamp(VkCommandBuffer cmd, VkPipelineStageFlagBits stage, uint32_t queryIndex);
    };
}