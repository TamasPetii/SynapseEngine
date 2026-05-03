#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"
#include "RenderConstants.h"
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

        uint32_t traditionalCmdOffsets[MaterialRenderType::Count] = { 0 };
        uint32_t traditionalCmdCounts[MaterialRenderType::Count] = { 0 };
        uint32_t meshletCmdOffsets[MaterialRenderType::Count] = { 0 };
        uint32_t meshletCmdCounts[MaterialRenderType::Count] = { 0 };        
    };
}