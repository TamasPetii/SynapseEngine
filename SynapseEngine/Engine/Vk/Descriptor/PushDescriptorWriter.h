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

namespace Syn::Vk {

    class SYN_API PushDescriptorWriter {
    public:
        PushDescriptorWriter() = default;
        ~PushDescriptorWriter() = default;

        PushDescriptorWriter& AddCombinedImageSampler(uint32_t binding, VkImageView view, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        PushDescriptorWriter& AddSampledImage(uint32_t binding, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        PushDescriptorWriter& AddStorageImage(uint32_t binding, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);
        PushDescriptorWriter& AddSampler(uint32_t binding, VkSampler sampler);
        PushDescriptorWriter& AddBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        void Push(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    private:
        struct ImageWriteData {
            uint32_t binding;
            VkDescriptorType type;
            VkDescriptorImageInfo info;
        };

        struct BufferWriteData {
            uint32_t binding;
            VkDescriptorType type;
            VkDescriptorBufferInfo info;
        };

        std::vector<ImageWriteData> _imageWrites;
        std::vector<BufferWriteData> _bufferWrites;
    };
}