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

#include "DirectionLightDrawGroup.h"

namespace Syn
{
    DirectionLightDrawGroup::DirectionLightDrawGroup(uint32_t frameCount)
    {
        cmdTemplate.vertexCount = 3;
        cmdTemplate.instanceCount = 0;
        cmdTemplate.firstVertex = 0;
        cmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        indirectBuffer.Initialize({ "DirectionLightDrawGroup_IndirectBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        indirectBuffer.UpdateCapacityAll(1);

        billboardSingleCmdBuffer.Initialize({ "DirectionLightDrawGroup_BillboardSingleCmdBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        billboardSingleCmdBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            billboardSingleCmdBuffer.Write(i, &billboardCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
        }
    }

    void DirectionLightDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {

    }
}