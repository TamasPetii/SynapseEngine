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

#include "TimestampQueryPool.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk
{
    TimestampQueryPool::TimestampQueryPool(uint32_t queryCount)
        : QueryPool(VK_QUERY_TYPE_TIMESTAMP, queryCount)
    {
    }

    void TimestampQueryPool::WriteTimestamp(VkCommandBuffer cmd, VkPipelineStageFlagBits stage, uint32_t queryIndex) {
        vkCmdWriteTimestamp(cmd, stage, _handle, queryIndex);
    }
}