#include "DescriptorUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk 
{
    VkDescriptorSetLayout DescriptorUtils::_emptyBufferLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout DescriptorUtils::_emptyStandardLayout = VK_NULL_HANDLE;

    void DescriptorUtils::Cleanup() {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();
        if (_emptyBufferLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, _emptyBufferLayout, nullptr);
            _emptyBufferLayout = VK_NULL_HANDLE;
        }
        if (_emptyStandardLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, _emptyStandardLayout, nullptr);
            _emptyStandardLayout = VK_NULL_HANDLE;
        }
    }

    VkDescriptorSetLayout DescriptorUtils::GetEmptyDescriptorSetLayout(bool useDescriptorBuffers) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        VkDescriptorSetLayout& targetLayout = useDescriptorBuffers ? _emptyBufferLayout : _emptyStandardLayout;

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
