#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"
#include "Engine/Mesh/MeshAllocationInfo.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Material/MaterialRenderType.h"
#include "IDrawGroup.h"
#include "Engine/Vk/Image/Image.h"

namespace Syn
{
    constexpr uint32_t SHADOW_LOD_BIAS = 1;
    constexpr uint32_t MAX_DIR_LIGHTS = 1;
    constexpr uint32_t CASCADES_PER_LIGHT = 4;
    constexpr uint32_t SHADOW_MULTIPLIER = MAX_DIR_LIGHTS * CASCADES_PER_LIGHT;

    constexpr uint32_t SHADOW_ATLAS_SIZE = 1024;
    constexpr uint32_t SHADOW_MIN_BLOCK_SIZE = 128;
    constexpr uint32_t SHADOW_GRID_SIZE = SHADOW_ATLAS_SIZE / SHADOW_MIN_BLOCK_SIZE;

    struct SYN_API DirectionLightShadowDrawGroup : public IDrawGroup
    {
        DirectionLightShadowDrawGroup(uint32_t frameCount);
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;
        
        /*
        - indirectBuffer: Same as ModelDrawGroup, need to copy to shadow buffer.
        - descriptorBuffer: Same as ModelDrawGroup, same gpu buffer
		- modelAllocBuffer: Same as ModelDrawGroup, same gpu buffer
		- meshAllocBuffer: Same as ModelDrawGroup, same gpu buffer, offset is multiplied by SHADOW_MULTIPLIER!!
        - drawCountBuffer: Same as ModelDrawGrop, same gpu buffer
		- traditionalCmdOffsets, traditionalCmdCounts, meshletCmdOffsets, meshletCmdCounts: Same as ModelDrawGroup
        */

        RenderBuffer instanceBuffer;
        RenderBuffer indirectBuffer;
        RenderBuffer computeCountBuffer;

        CpuData<VkDrawIndirectCommand> traditionalCmds;
        CpuData<VkDrawMeshTasksIndirectCommandEXT> meshletCmds;

        CpuData<uint32_t> instances;
        CpuData<uint32_t> paddedTraditionalCounts;
        CpuData<uint32_t> paddedMeshletCounts;

        CpuData<uint32_t> visibleLights;
        uint32_t visibleLightCount = 0;

        CpuData<uint32_t> visibleChunkIds;
        std::atomic<uint32_t> visibleChunkCount{ 0 };

        VkDispatchIndirectCommand dispatchCmdTemplate{};

        std::vector<std::unique_ptr<Vk::Image>> shadowAtlas;
    };
}