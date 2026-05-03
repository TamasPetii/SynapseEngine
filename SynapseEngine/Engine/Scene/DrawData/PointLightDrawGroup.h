#pragma once
#include "IDrawGroup.h"

namespace Syn
{
    struct SYN_API PointLightDrawGroup : public IDrawGroup {
        PointLightDrawGroup(uint32_t frameCount);
		virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer indirectBuffer;
        RenderBuffer sphereSingleCmdBuffer;
        RenderBuffer aabbSingleCmdBuffer;
        RenderBuffer billboardSingleCmdBuffer;

        CpuData<uint32_t> instances;
        VkDrawIndirectCommand cmdTemplate{};
    };
}