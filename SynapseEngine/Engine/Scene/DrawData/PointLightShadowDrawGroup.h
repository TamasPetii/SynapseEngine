#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"
#include "Engine/Mesh/MeshAllocationInfo.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Material/MaterialRenderType.h"
#include "IDrawGroup.h"
#include "Engine/Vk/Image/Image.h"
#include <bit>
#include <vector>
#include <atomic>
#include <memory>

namespace Syn
{
    constexpr uint32_t POINT_SHADOW_LOD_BIAS = 1;
    constexpr uint32_t POINT_SHADOW_MULTIPLIER = 6;

    constexpr uint32_t POINT_SHADOW_ATLAS_SIZE = 4096;
    constexpr uint32_t POINT_SHADOW_MIN_BLOCK_SIZE = 64;
    constexpr uint32_t POINT_SHADOW_GRID_SIZE = POINT_SHADOW_ATLAS_SIZE / POINT_SHADOW_MIN_BLOCK_SIZE;
    constexpr uint32_t POINT_SHADOW_HIZ_MIP_LEVELS = std::countr_zero(POINT_SHADOW_MIN_BLOCK_SIZE) + 1;

    struct PointShadowInstancePayload {
        uint32_t entityData; // [Bit 31: FullyInside] [Bit 0-30: EntityID]
        uint32_t lightIndex; // [Bit 31-29: Side] [Bit 28-0: LightIndex]
    };

    struct SYN_API PointLightShadowDrawGroup : public IDrawGroup
    {
        PointLightShadowDrawGroup(uint32_t frameCount);
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer instanceBuffer;
        RenderBuffer unsortedInstanceBuffer;
        RenderBuffer indirectBuffer;
        RenderBuffer descriptorBuffer;
        RenderBuffer modelCullingIndirectDispatchBuffer;
        RenderBuffer finalizeDispatchBuffer;

        RenderBuffer atlasRadixSortTempBuffer;
        RenderBuffer radixSortTempBuffer;
        RenderBuffer drawCallKeyBuffer;
        RenderBuffer sortValuesBuffer;

        RenderBuffer modelDispatchBuffer;
        RenderBuffer staticChunkDispatchBuffer;
        RenderBuffer mortonChunkDispatchBuffer;

        RenderBuffer visibleCountDispatchBuffer;
        RenderBuffer visibleMeshCountDispatchBuffer;
        RenderBuffer gridLookupBuffer;

        CpuData<uint32_t> gridLookupData;
        CpuData<MeshDrawDescriptor> shadowDescriptors;
        CpuData<VkDrawIndirectCommand> traditionalCmds;
        CpuData<VkDrawMeshTasksIndirectCommandEXT> meshletCmds;

        CpuData<PointShadowInstancePayload> instances;
        std::atomic<uint32_t> appendedInstanceCount{ 0 };

        CpuData<uint32_t> visibleLights;
        uint32_t visibleLightCount = 0;

        CpuData<uint32_t> visibleChunkIds;
        std::atomic<uint32_t> visibleChunkCount{ 0 };

        VkDispatchIndirectCommand dispatchCmdTemplate{};
        uint32_t totalCommandCount = 0;

        std::vector<std::unique_ptr<Vk::Image>> shadowAtlas;
        std::vector<std::unique_ptr<Vk::Image>> shadowDepthPyramid;
    };
}