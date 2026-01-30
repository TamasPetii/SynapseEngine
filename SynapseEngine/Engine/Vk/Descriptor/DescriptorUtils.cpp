#include "DescriptorUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {
    VkDescriptorSetLayout DescriptorUtils::GetEmptyDescriptorSetLayout() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        static VkDescriptorSetLayout emptyLayout = VK_NULL_HANDLE;

        if (emptyLayout == VK_NULL_HANDLE) {
            VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            layoutInfo.bindingCount = 0;
            layoutInfo.pBindings = nullptr;
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

            if (vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &emptyLayout) != VK_SUCCESS) {
                return VK_NULL_HANDLE;
            }
        }

        return emptyLayout;
    }
}

