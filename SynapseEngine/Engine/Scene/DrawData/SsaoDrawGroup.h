#pragma once
#include "IDrawGroup.h"
#include <glm/glm.hpp>
#include "Engine/Registry/Entity.h"

namespace Syn
{
    struct SYN_API SsaoKernel {
        glm::vec4 samples[64];
    };

    struct SYN_API SsaoDrawGroup : public IDrawGroup {
        SsaoDrawGroup(uint32_t frameCount);
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer kernelBuffer;
    };
}