#include "QueryPool.h"
#include "Engine/SynMacro.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk
{
    QueryPool::QueryPool(VkQueryType type, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStats)
        : _type(type), _queryCount(queryCount)
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkQueryPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
        poolInfo.queryType = type;
        poolInfo.queryCount = queryCount;
        poolInfo.pipelineStatistics = pipelineStats;

        SYN_VK_ASSERT_MSG(vkCreateQueryPool(device->Handle(), &poolInfo, nullptr, &_handle), "Failed to create Query Pool");
    }

    QueryPool::~QueryPool() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::GetVkContext()->GetDevice();
            vkDestroyQueryPool(device->Handle(), _handle, nullptr);
        }
    }

    void QueryPool::Reset(VkCommandBuffer cmd, uint32_t firstQuery, uint32_t count) {
        vkCmdResetQueryPool(cmd, _handle, firstQuery, count);
    }

    bool QueryPool::GetResults(uint32_t firstQuery, uint32_t count, std::vector<uint64_t>& outResults, bool wait) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        if (outResults.size() < count) {
            outResults.resize(count);
        }

        VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT;

        if (wait) {
            flags |= VK_QUERY_RESULT_WAIT_BIT;
        }

        VkResult result = vkGetQueryPoolResults(
            device->Handle(),
            _handle,
            firstQuery,
            count,
            outResults.size() * sizeof(uint64_t),
            outResults.data(),
            sizeof(uint64_t),
            flags
        );

        return result == VK_SUCCESS;
    }
}