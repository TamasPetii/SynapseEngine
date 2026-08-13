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

#include "SpotLightDrawGroup.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn
{
    SpotLightDrawGroup::SpotLightDrawGroup(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);
        auto cone = modelManager->GetResource(MeshSourceNames::Cone);
        auto pyramid = modelManager->GetResource(MeshSourceNames::ProxyPyramid);

        VkDrawIndirectCommand sphereCmdTemplate{};
        sphereCmdTemplate.vertexCount = sphere->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        sphereCmdTemplate.instanceCount = 0;
        sphereCmdTemplate.firstVertex = sphere->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        sphereCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand aabbCmdTemplate{};
        aabbCmdTemplate.vertexCount = cube->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        aabbCmdTemplate.instanceCount = 0;
        aabbCmdTemplate.firstVertex = cube->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        aabbCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand pyramidCmdTemplate{};
        pyramidCmdTemplate.vertexCount = pyramid->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        pyramidCmdTemplate.instanceCount = 0;
        pyramidCmdTemplate.firstVertex = pyramid->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        pyramidCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand coneCmdTemplate{};
        coneCmdTemplate.vertexCount = cone->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        coneCmdTemplate.instanceCount = 0;
        coneCmdTemplate.firstVertex = cone->cpuData.baseDrawCommands[0].traditionalCmd.firstVertex;
        coneCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

        cmdTemplate = pyramidCmdTemplate;

        VkBufferUsageFlags indirectUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        indirectBuffer.Initialize({ "SpotLightDrawGroup_IndirectBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        indirectBuffer.UpdateCapacityAll(1);

        sphereSingleCmdBuffer.Initialize({ "SpotLightDrawGroup_SphereSingleCmdBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        sphereSingleCmdBuffer.UpdateCapacityAll(1);

        coneSingleCmdBuffer.Initialize({ "SpotLightDrawGroup_ConeSingleCmdBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        coneSingleCmdBuffer.UpdateCapacityAll(1);

		pyramidSingleCmdBuffer.Initialize({ "SpotLightDrawGroup_PyramidSingleCmdBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
		pyramidSingleCmdBuffer.UpdateCapacityAll(1);

        aabbSingleCmdBuffer.Initialize({ "SpotLightDrawGroup_AABBSingleCmdBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        aabbSingleCmdBuffer.UpdateCapacityAll(1);

        billboardSingleCmdBuffer.Initialize({ "SpotLightDrawGroup_BillboardSingleCmdBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), indirectUsage });
        billboardSingleCmdBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            sphereSingleCmdBuffer.Write(i, &sphereCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            aabbSingleCmdBuffer.Write(i, &aabbCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            billboardSingleCmdBuffer.Write(i, &billboardCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
			coneSingleCmdBuffer.Write(i, &coneCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
			pyramidSingleCmdBuffer.Write(i, &pyramidCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
        }
    }

	void SpotLightDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        indirectBuffer.RecordSync(cmd, frameIndex);
    }
}