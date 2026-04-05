#pragma once
#include "IProfiler.h"
#include <vulkan/vulkan.h>

namespace Syn {

    class SYN_API IGpuProfiler : public IProfiler {
    public:
        virtual void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) = 0;
        virtual uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& name) = 0;
        virtual void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t measurementIndex) = 0;
    };

}