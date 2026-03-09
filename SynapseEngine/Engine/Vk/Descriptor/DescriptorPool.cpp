#include "DescriptorPool.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn::Vk {

    DescriptorPool::DescriptorPool(uint32_t maxSets, std::span<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags poolFlags) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = poolFlags;

        SYN_VK_ASSERT_MSG(vkCreateDescriptorPool(device->Handle(), &poolInfo, nullptr, &_handle), "Failed to create Descriptor Pool");
    }

    DescriptorPool::~DescriptorPool() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::GetVkContext()->GetDevice();
            vkDestroyDescriptorPool(device->Handle(), _handle, nullptr);
        }
    }

    VkDescriptorSet DescriptorPool::AllocateSet(VkDescriptorSetLayout layout) {
        return AllocateSets({ &layout, 1 }).front();
    }

    std::vector<VkDescriptorSet> DescriptorPool::AllocateSets(std::span<const VkDescriptorSetLayout> layouts) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = _handle;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(layouts.size());
        SYN_VK_ASSERT_MSG(vkAllocateDescriptorSets(device->Handle(), &allocInfo, sets.data()), "Failed to allocate Descriptor Sets");
        return sets;
    }

    VkDescriptorSet DescriptorPool::AllocateVariableSet(VkDescriptorSetLayout layout, uint32_t variableCount) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkDescriptorSetVariableDescriptorCountAllocateInfo variableInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
        variableInfo.descriptorSetCount = 1;
        variableInfo.pDescriptorCounts = &variableCount;

        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.pNext = &variableInfo;
        allocInfo.descriptorPool = _handle;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet set = VK_NULL_HANDLE;
        SYN_VK_ASSERT_MSG(vkAllocateDescriptorSets(device->Handle(), &allocInfo, &set), "Failed to allocate Variable Descriptor Set");
        return set;
    }
}