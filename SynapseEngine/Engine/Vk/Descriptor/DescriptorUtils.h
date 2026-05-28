#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    class SYN_API DescriptorUtils {
    public:
        static void Cleanup();
        static VkDescriptorSetLayout GetEmptyDescriptorSetLayout(bool useDescriptorBuffers);
    private:
        static VkDescriptorSetLayout _emptyBufferLayout;
        static VkDescriptorSetLayout _emptyStandardLayout;
     };
}