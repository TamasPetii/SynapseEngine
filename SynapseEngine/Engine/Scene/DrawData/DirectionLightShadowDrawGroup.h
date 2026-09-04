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
#include "Engine/Vk/Image/Image.h"
#include <bit>
#include <vector>
#include <atomic>

namespace Syn
{
    constexpr uint32_t SHADOW_LOD_BIAS = 1;
    constexpr uint32_t MAX_DIR_LIGHTS = 4;
    constexpr uint32_t CASCADES_PER_LIGHT = 4;
    constexpr uint32_t SHADOW_MULTIPLIER = MAX_DIR_LIGHTS * CASCADES_PER_LIGHT;

    constexpr uint32_t SHADOW_ATLAS_SIZE = 4096;
    constexpr uint32_t SHADOW_MIN_BLOCK_SIZE = 2048;
    constexpr uint32_t SHADOW_GRID_SIZE = SHADOW_ATLAS_SIZE / SHADOW_MIN_BLOCK_SIZE;
    constexpr uint32_t SHADOW_HIZ_MIP_LEVELS = std::countr_zero(SHADOW_MIN_BLOCK_SIZE) + 1;

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

        RenderBuffer modelDispatchBuffer;
        RenderBuffer staticChunkDispatchBuffer;
        RenderBuffer mortonChunkDispatchBuffer;

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
		uint32_t totalCommandCount = 0;

        std::vector<std::unique_ptr<Vk::Image>> shadowAtlas;
        std::vector<std::unique_ptr<Vk::Image>> shadowDepthPyramid;
        std::vector<std::unique_ptr<Vk::Image>> shadowColorAtlas;

        std::vector<std::unique_ptr<Vk::Image>> staticShadowAtlas;
        std::vector<std::unique_ptr<Vk::Image>> staticShadowColorAtlas;

        std::atomic<uint32_t> staticCacheModelVersion{ 0 };
        std::atomic<uint32_t> staticModelSwapVersion{ 0 };
        std::atomic<uint32_t> processedStaticModelSwapVersion{ 0 };
        RenderBuffer animatedStaticDispatchBuffer;
    };
}