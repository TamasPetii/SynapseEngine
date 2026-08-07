#pragma once
#include "../VkCommon.h"
#include <vector>
#include <utility>
#include <span>

namespace Syn::Vk 
{
    class DescriptorBuffer;

    class SYN_API DescriptorUtils {
    public:
        static void Cleanup();
        static VkDescriptorSetLayout GetEmptyDescriptorSetLayout(bool useDescriptorBuffers);
        static void BindMultipleBuffer(
            VkCommandBuffer cmd,
            VkPipelineLayout pipelineLayout,
            VkPipelineBindPoint bindPoint,
            std::span<const std::pair<uint32_t, DescriptorBuffer*>> sets
        );
    private:
        static VkDescriptorSetLayout _emptyBufferLayout;
        static VkDescriptorSetLayout _emptyStandardLayout;
     };
}