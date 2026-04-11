#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"

namespace Syn
{
    struct SpotLightDrawGroup {
        SpotLightDrawGroup(uint32_t frameCount);

        RenderBuffer indirectBuffer;
        RenderBuffer sphereSingleCmdBuffer;
        RenderBuffer aabbSingleCmdBuffer;
        RenderBuffer billboardSingleCmdBuffer;

        CpuData<uint32_t> instances;
        VkDrawIndirectCommand cmdTemplate{};
    };
}