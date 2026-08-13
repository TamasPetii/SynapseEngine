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

#include "ModelDrawGroup.h"
#include "Engine/Mesh/ModelManager.h"

namespace Syn
{
    ModelDrawGroup::ModelDrawGroup(uint32_t frameCount)
    {
        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        paddedTraditionalCounts.AssignZero(16);
        paddedMeshletCounts.AssignZero(16);

        instances.AssignZero(1);
        traditionalCmds.AssignZero(1);
        meshletCmds.AssignZero(1);
        descriptors.AssignZero(1);
        meshAllocations.AssignZero(1);
        modelAllocations.AssignZero(1);

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        instanceBuffer.Initialize({ "ModelDrawGroup_InstanceBuffer", BufferStrategy::Hybrid, frameCount, sizeof(uint32_t), storageUsage, 16384, 32768});
        instanceBuffer.UpdateCapacityAll(1);

		//VkDrawIndirectCommand + VkDrawMeshTasksIndirectCommandEXT
        indirectBuffer.Initialize({ "ModelDrawGroup_IndirectBuffer", BufferStrategy::Hybrid, frameCount, sizeof(VkDrawIndirectCommand) * MaterialRenderType::MaterialRenderTypeCount * 2, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        descriptorBuffer.Initialize({ "ModelDrawGroup_DescriptorBuffer", BufferStrategy::Hybrid, frameCount, sizeof(MeshDrawDescriptor), storageUsage, 1024, 2048 });
        descriptorBuffer.UpdateCapacityAll(1);

        modelAllocBuffer.Initialize({ "ModelDrawGroup_ModelAllocBuffer", BufferStrategy::Hybrid, frameCount, sizeof(ModelAllocationInfo), storageUsage, 1024, 2048 });
        modelAllocBuffer.UpdateCapacityAll(1);

        meshAllocBuffer.Initialize({ "ModelDrawGroup_MeshAllocBuffer", BufferStrategy::Hybrid, frameCount, sizeof(MeshAllocationInfo), storageUsage, 1024, 2048 });
        meshAllocBuffer.UpdateCapacityAll(1);

        materialIndexBuffer.Initialize({ "ModelDrawGroup_MaterialIndexBuffer", BufferStrategy::Hybrid, frameCount, sizeof(int32_t), storageUsage, 4096, 8192 });
        materialIndexBuffer.UpdateCapacityAll(1);

        pipelineIndexBuffer.Initialize({ "ModelDrawGroup_PipelineIndexBuffer", BufferStrategy::Hybrid, frameCount, sizeof(int32_t), storageUsage, 4096, 8192 });
        pipelineIndexBuffer.UpdateCapacityAll(1);

        drawCountBuffer.Initialize({ "ModelDrawGroup_DrawCountBuffer", BufferStrategy::MappedOnly, frameCount, sizeof(uint32_t), indirectStorageUsage, 1, 1 });
        drawCountBuffer.UpdateCapacityAll(MaterialRenderType::MaterialRenderTypeCount * 2);

        computeCountBuffer.Initialize({ "ModelDrawGroup_ComputeCountBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        computeCountBuffer.UpdateCapacityAll(1);
    }

    void ModelDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        instanceBuffer.RecordSync(cmd, frameIndex);
        indirectBuffer.RecordSync(cmd, frameIndex);
        descriptorBuffer.RecordSync(cmd, frameIndex);
        modelAllocBuffer.RecordSync(cmd, frameIndex);
        meshAllocBuffer.RecordSync(cmd, frameIndex);
        materialIndexBuffer.RecordSync(cmd, frameIndex);
        pipelineIndexBuffer.RecordSync(cmd, frameIndex);
    }
}