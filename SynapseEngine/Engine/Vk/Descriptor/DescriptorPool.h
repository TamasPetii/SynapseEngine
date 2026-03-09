#pragma once
#include "../VkCommon.h"
#include <vector>
#include <span>

namespace Syn::Vk {

    class SYN_API DescriptorPool {
    public:
        DescriptorPool(uint32_t maxSets, std::span<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags poolFlags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        VkDescriptorPool Handle() const { return _handle; }

        VkDescriptorSet AllocateSet(VkDescriptorSetLayout layout);
        std::vector<VkDescriptorSet> AllocateSets(std::span<const VkDescriptorSetLayout> layouts);

        VkDescriptorSet AllocateVariableSet(VkDescriptorSetLayout layout, uint32_t variableCount);
    private:
        VkDescriptorPool _handle = VK_NULL_HANDLE;
    };
}