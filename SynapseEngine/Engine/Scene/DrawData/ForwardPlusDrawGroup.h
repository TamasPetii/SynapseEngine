#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn
{
    struct SYN_API ForwardPlusDrawGroup {
        ForwardPlusDrawGroup(uint32_t frameCount);

        RenderBuffer tileGridBuffer;
        RenderBuffer clusterListBuffer;
        RenderBuffer clusterCountBuffer;

        RenderBuffer pointLightIndexBuffer;
        RenderBuffer spotLightIndexBuffer;
        RenderBuffer lightCounterBuffer;

        uint32_t tileSize = ComputeGroupSize::Image16D;
        uint32_t maxClusters = 0;

        void RecordSync(VkCommandBuffer cmd, uint32_t frameIndex);
        void CheckResize(uint32_t width, uint32_t height, uint32_t frameIndex);
    };
}