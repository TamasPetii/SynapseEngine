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
#include "IDrawGroup.h"
#include <glm/glm.hpp>
#include "Engine/Registry/Entity.h"
#include <vector>
#include <atomic>

namespace Syn
{
    struct SYN_API ChunkDataGPU {
        glm::vec3 minBounds;
        uint32_t  firstEntityIndex;
        glm::vec3 maxBounds;
        uint32_t  entityCount;
    };

    struct SYN_API SceneAABB {
        uint32_t minX, minY, minZ;
        uint32_t maxX, maxY, maxZ;
    };

    struct SYN_API ChunkDrawGroup : public IDrawGroup {
        ChunkDrawGroup(uint32_t frameCount);
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer chunkDataBuffer;
        RenderBuffer chunkVisibilityBuffer;
        RenderBuffer chunkAabbSingleCmdBuffer;
        RenderBuffer chunkIndirectDispatchBuffer;

        RenderBuffer sceneAabbBuffer;
        RenderBuffer mortonRadixSortTempBuffer;
        RenderBuffer mortonIndirectDispatchBuffer;
        RenderBuffer mortonIndirectDrawBuffer;
        RenderBuffer mortonAabbSingleCmdBuffer;
        RenderBuffer mortonChunkVisibleIndirectDispatchBuffer;

        std::vector<uint32_t> visibleChunkIds;
        std::atomic<uint32_t> visibleChunkCount{ 0 };

        std::vector<ChunkDataGPU> chunks;
        std::atomic<uint32_t> chunkCounter{ 0 };

        VkDrawIndirectCommand wireframeCmdTemplate{};
        VkDispatchIndirectCommand dispatchCmdTemplate{};
    };
}