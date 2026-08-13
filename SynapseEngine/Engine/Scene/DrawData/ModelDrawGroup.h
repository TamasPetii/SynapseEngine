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
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"
#include "Engine/Mesh/MeshAllocationInfo.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Material/MaterialRenderType.h"
#include "IDrawGroup.h"

namespace Syn
{
    struct SYN_API ModelDrawGroup : public IDrawGroup {
        ModelDrawGroup(uint32_t frameCount);
		virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer instanceBuffer;
        RenderBuffer indirectBuffer;
        RenderBuffer descriptorBuffer;
        RenderBuffer modelAllocBuffer;
        RenderBuffer meshAllocBuffer;
        RenderBuffer materialIndexBuffer;
        RenderBuffer pipelineIndexBuffer;
        RenderBuffer drawCountBuffer;
        RenderBuffer computeCountBuffer;

        CpuData<uint32_t> instances;
        CpuData<VkDrawIndirectCommand> traditionalCmds;
        CpuData<VkDrawMeshTasksIndirectCommandEXT> meshletCmds;
        CpuData<MeshDrawDescriptor> descriptors;
        CpuData<MeshAllocationInfo> meshAllocations;
        CpuData<ModelAllocationInfo> modelAllocations;

        CpuData<uint32_t> paddedTraditionalCounts;
        CpuData<uint32_t> paddedMeshletCounts;

        uint32_t activeDescriptorCount = 0;
        uint32_t activeTraditionalCount = 0;
        uint32_t activeMeshletCount = 0;
        uint32_t totalAllocatedInstances = 0;
        size_t requiredMaterialBufferSize = 0;

        uint64_t maxPossibleVertices = 0;
        uint64_t maxPossibleIndices = 0;
        uint64_t maxPossibleTriangles = 0;

        uint32_t traditionalCmdOffsets[MaterialRenderType::MaterialRenderTypeCount] = { 0 };
        uint32_t traditionalCmdCounts[MaterialRenderType::MaterialRenderTypeCount] = { 0 };
        uint32_t meshletCmdOffsets[MaterialRenderType::MaterialRenderTypeCount] = { 0 };
        uint32_t meshletCmdCounts[MaterialRenderType::MaterialRenderTypeCount] = { 0 };

        VkDispatchIndirectCommand dispatchCmdTemplate{};
    };
}