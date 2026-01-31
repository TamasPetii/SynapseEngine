#pragma once
#include "../VkCommon.h"
#include "QueryPool.h"

namespace Syn::Vk
{
    class SYN_API OcclusionQueryPool : public QueryPool {
    public:
        OcclusionQueryPool(uint32_t queryCount);
        void BeginQuery(VkCommandBuffer cmd, uint32_t queryIndex, VkQueryControlFlags flags = 0);
        void EndQuery(VkCommandBuffer cmd, uint32_t queryIndex);
    };
}