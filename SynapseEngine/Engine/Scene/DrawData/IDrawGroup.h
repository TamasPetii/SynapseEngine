#pragma once
#include "CpuData.h"
#include "RenderConstants.h"
#include "Engine/Utils/RenderBuffer.h"

namespace Syn
{
    struct SYN_API IDrawGroup {
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) = 0;
    };
}