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