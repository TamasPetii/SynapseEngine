#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"

namespace Syn
{
    struct DirectionLightDrawGroup {
        DirectionLightDrawGroup(uint32_t frameCount);

        RenderBuffer indirectBuffer;
        RenderBuffer billboardSingleCmdBuffer;

        CpuData<uint32_t> instances;
        VkDrawIndirectCommand cmdTemplate{};
    };
}