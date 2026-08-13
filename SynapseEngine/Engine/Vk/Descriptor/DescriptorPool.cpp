// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "DescriptorPool.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn::Vk {

    DescriptorPool::DescriptorPool(uint32_t maxSets, std::span<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags poolFlags) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = poolFlags;

        SYN_VK_ASSERT_MSG(vkCreateDescriptorPool(device->Handle(), &poolInfo, nullptr, &_handle), "Failed to create Descriptor Pool");
    }

    DescriptorPool::~DescriptorPool() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
            vkDestroyDescriptorPool(device->Handle(), _handle, nullptr);
        }
    }

    VkDescriptorSet DescriptorPool::AllocateSet(VkDescriptorSetLayout layout) {
        return AllocateSets({ &layout, 1 }).front();
    }

    std::vector<VkDescriptorSet> DescriptorPool::AllocateSets(std::span<const VkDescriptorSetLayout> layouts) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.descriptorPool = _handle;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
        allocInfo.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(layouts.size());
        SYN_VK_ASSERT_MSG(vkAllocateDescriptorSets(device->Handle(), &allocInfo, sets.data()), "Failed to allocate Descriptor Sets");
        return sets;
    }

    VkDescriptorSet DescriptorPool::AllocateVariableSet(VkDescriptorSetLayout layout, uint32_t variableCount) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

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