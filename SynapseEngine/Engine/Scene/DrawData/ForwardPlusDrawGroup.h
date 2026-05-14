#pragma once
#include "IDrawGroup.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn
{
    struct SYN_API ForwardPlusDrawGroup : public IDrawGroup 
    {
        ForwardPlusDrawGroup(uint32_t frameCount);	
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;
        void CheckResize(uint32_t width, uint32_t height, uint32_t frameIndex);

        RenderBuffer tileGridBuffer;
        RenderBuffer clusterListBuffer;
        RenderBuffer clusterCountBuffer;

        RenderBuffer pointLightIndexBuffer;
        RenderBuffer spotLightIndexBuffer;

        uint32_t maxClusters = 0;     
        uint32_t tileSize = ComputeGroupSize::Image16D;
    };
}