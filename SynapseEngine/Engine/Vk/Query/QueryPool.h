#pragma once
#include "../VkCommon.h"
#include <vector>

namespace Syn::Vk
{
    class SYN_API QueryPool {
    public:
        virtual ~QueryPool();

        QueryPool(const QueryPool&) = delete;
        QueryPool& operator=(const QueryPool&) = delete;

        void Reset(VkCommandBuffer cmd, uint32_t firstQuery, uint32_t count);
        bool GetResults(uint32_t firstQuery, uint32_t count, std::vector<uint64_t>& outResults, bool wait = false);

        VkQueryPool Handle() const { return _handle; }
        uint32_t GetCount() const { return _queryCount; }
        VkQueryType GetType() const { return _type; }
    protected:
        QueryPool(VkQueryType type, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStats = 0);
    protected:
        VkQueryType _type;
        uint32_t _queryCount = 0;
        VkQueryPool _handle = VK_NULL_HANDLE;
    };
}