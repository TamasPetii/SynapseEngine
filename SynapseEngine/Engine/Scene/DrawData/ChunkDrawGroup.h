#pragma once
#include "IDrawGroup.h"
#include <glm/glm.hpp>
#include "Engine/Registry/Entity.h"

namespace Syn
{
    struct SYN_API ChunkDataGPU {
        glm::vec3 minBounds;
        uint32_t  firstEntityIndex;
        glm::vec3 maxBounds;
        uint32_t  entityCount;
    };

    struct SYN_API ChunkDrawGroup : public IDrawGroup {
        ChunkDrawGroup(uint32_t frameCount);
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer chunkDataBuffer;
        RenderBuffer chunkVisibilityBuffer;
        RenderBuffer aabbSingleCmdBuffer;
        RenderBuffer indirectDispatchBuffer;

        std::vector<ChunkDataGPU> chunks;
        std::vector<uint32_t> visibleChunkIds;

        std::atomic<uint32_t> visibleChunkCount{ 0 };
        std::atomic<uint32_t> chunkCounter{ 0 };

        VkDrawIndirectCommand wireframeCmdTemplate{};
        VkDispatchIndirectCommand dispatchCmdTemplate{};
    };
}