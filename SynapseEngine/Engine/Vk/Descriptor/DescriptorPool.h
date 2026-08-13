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