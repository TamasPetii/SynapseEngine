#include "DescriptorUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {
    VkDescriptorSetLayout DescriptorUtils::GetEmptyDescriptorSetLayout(bool useDescriptorBuffers) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        static VkDescriptorSetLayout emptyBufferLayout = VK_NULL_HANDLE;
        static VkDescriptorSetLayout emptyStandardLayout = VK_NULL_HANDLE;

        VkDescriptorSetLayout& targetLayout = useDescriptorBuffers ? emptyBufferLayout : emptyStandardLayout;

        if (targetLayout == VK_NULL_HANDLE) {
            VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            layoutInfo.bindingCount = 0;
            layoutInfo.pBindings = nullptr;
            layoutInfo.flags = useDescriptorBuffers ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT : 0;

            if (vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &targetLayout) != VK_SUCCESS) {
                return VK_NULL_HANDLE;
            }
        }

        return targetLayout;
    }
}
