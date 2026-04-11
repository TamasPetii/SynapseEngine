#pragma once
#include "Engine/Utils/RenderBuffer.h"
#include "CpuData.h"
#include "RenderConstants.h"

namespace Syn
{
    struct DebugDrawGroup {
        DebugDrawGroup(uint32_t frameCount);

        RenderBuffer modelAabbIndirectBuffer;
        RenderBuffer modelSphereIndirectBuffer;
        RenderBuffer meshletAabbIndirectBuffer;
        RenderBuffer meshletSphereIndirectBuffer;
        RenderBuffer instanceBuffer;

        CpuData<VkDrawIndirectCommand> modelAabbCmds;
        CpuData<VkDrawIndirectCommand> modelSphereCmds;

        VkDrawIndirectCommand modelAabbCmdTemplate{};
        VkDrawIndirectCommand modelSphereCmdTemplate{};

        uint32_t totalMaxMeshletInstances = 0;
    };
}