#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    class SYN_API DescriptorUtils {
    public:
        static VkDescriptorSetLayout GetEmptyDescriptorSetLayout(bool useDescriptorBuffers);
     };
}