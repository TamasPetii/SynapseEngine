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

#include "DescriptorUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "DescriptorBuffer.h"

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

    void DescriptorUtils::BindMultipleBuffer(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, VkPipelineBindPoint bindPoint, std::span<const std::pair<uint32_t, DescriptorBuffer*>> sets)
    {
        if (sets.empty()) return;

        std::vector<VkDescriptorBufferBindingInfoEXT> bindingInfos;
        bindingInfos.reserve(sets.size());

        for (const auto& [setIndex, buffer] : sets) {
            VkDescriptorBufferBindingInfoEXT info{ VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT };
            info.address = buffer->GetDeviceAddress();
            info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
            bindingInfos.push_back(info);
        }

        vkCmdBindDescriptorBuffersEXT(cmd, static_cast<uint32_t>(bindingInfos.size()), bindingInfos.data());

        for (uint32_t i = 0; i < sets.size(); ++i) {
            uint32_t setIndex = sets[i].first;
            uint32_t bufferIndex = i;
            VkDeviceSize offset = 0;

            vkCmdSetDescriptorBufferOffsetsEXT(cmd, bindPoint, pipelineLayout, setIndex, 1, &bufferIndex, &offset);
        }
    }
}
