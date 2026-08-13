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