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
		virtual uint32_t GetStride() const { return sizeof(uint64_t); }
    protected:
        VkQueryType _type;
        uint32_t _queryCount = 0;
        VkQueryPool _handle = VK_NULL_HANDLE;
    };
}