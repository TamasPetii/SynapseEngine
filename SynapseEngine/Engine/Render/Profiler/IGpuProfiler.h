#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace Syn {

    class SYN_API IGpuProfiler {
    public:
        virtual ~IGpuProfiler() = default;

        virtual void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) = 0;
        virtual uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& name) = 0;
        virtual void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t measurementIndex) = 0;
        virtual void ResolveFrame(uint32_t frameIndex) = 0;
        virtual const std::unordered_map<std::string, float>& GetTimings(uint32_t frameIndex) const = 0;
    };

}