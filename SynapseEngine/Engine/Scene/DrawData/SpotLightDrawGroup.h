#pragma once
#include "IDrawGroup.h"

namespace Syn
{
    struct SYN_API SpotLightDrawGroup : public IDrawGroup {
        SpotLightDrawGroup(uint32_t frameCount);
		virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer indirectBuffer;
        RenderBuffer aabbSingleCmdBuffer;
        RenderBuffer sphereSingleCmdBuffer;
        RenderBuffer coneSingleCmdBuffer;
        RenderBuffer pyramidSingleCmdBuffer;
        RenderBuffer billboardSingleCmdBuffer;

        CpuData<uint32_t> instances;
        VkDrawIndirectCommand cmdTemplate{};
    };
}