// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "QueryPool.h"
#include "Engine/SynMacro.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk
{
    QueryPool::QueryPool(VkQueryType type, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStats)
        : _type(type), _queryCount(queryCount)
    {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        VkQueryPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
        poolInfo.queryType = type;
        poolInfo.queryCount = queryCount;
        poolInfo.pipelineStatistics = pipelineStats;

        SYN_VK_ASSERT_MSG(vkCreateQueryPool(device->Handle(), &poolInfo, nullptr, &_handle), "Failed to create Query Pool");
    }

    QueryPool::~QueryPool() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
            vkDestroyQueryPool(device->Handle(), _handle, nullptr);
        }
    }

    void QueryPool::Reset(VkCommandBuffer cmd, uint32_t firstQuery, uint32_t count) {
        vkCmdResetQueryPool(cmd, _handle, firstQuery, count);
    }

    bool QueryPool::GetResults(uint32_t firstQuery, uint32_t count, std::vector<uint64_t>& outResults, bool wait) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        uint32_t stride = GetStride();
        uint32_t statsPerQuery = stride / sizeof(uint64_t);
        uint32_t requiredElements = count * statsPerQuery;

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
            stride,
            flags
        );

        return result == VK_SUCCESS;
    }
}