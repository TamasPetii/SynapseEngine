#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Syn
{
    struct SYN_API RenderPassStats {
        std::string groupName;
        std::string passName;

        uint64_t inputAssemblyVertices = 0;
        uint64_t inputAssemblyPrimitives = 0;
        uint64_t vertexShaderInvocations = 0;
        uint64_t clippingInvocations = 0;
        uint64_t clippingPrimitives = 0;
        uint64_t fragmentShaderInvocations = 0;
        uint64_t taskShaderInvocations = 0;
        uint64_t meshShaderInvocations = 0;
    };

    class SYN_API IRenderStatCollector {
    public:
        virtual ~IRenderStatCollector() = default;

        virtual void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) = 0;
        virtual uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& groupName, const std::string& name) = 0;
        virtual void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t queryIndex) = 0;

        virtual void ResolveFrame(uint32_t frameIndex) = 0;
        virtual const std::vector<RenderPassStats>& GetStats(uint32_t frameIndex) const = 0;
    };
}