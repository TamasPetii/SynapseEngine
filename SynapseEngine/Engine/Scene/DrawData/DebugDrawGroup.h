#pragma once
#include "IDrawGroup.h"

namespace Syn
{
    struct SYN_API DebugDrawGroup : public IDrawGroup {
        DebugDrawGroup(uint32_t frameCount);
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

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