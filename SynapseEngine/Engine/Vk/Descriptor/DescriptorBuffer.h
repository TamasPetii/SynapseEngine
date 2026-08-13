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
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>
#include <mutex>

namespace Syn::Vk {

    struct SYN_API StaleDescriptorBuffers {
        std::shared_ptr<Vk::Buffer> mapped;
        std::shared_ptr<Vk::Buffer> gpu;
    };

    class SYN_API DescriptorBuffer {
    public:
        DescriptorBuffer(VkDescriptorSetLayout layout);
        ~DescriptorBuffer() = default;

        DescriptorBuffer(const DescriptorBuffer&) = delete;
        DescriptorBuffer& operator=(const DescriptorBuffer&) = delete;

        Buffer* GetBuffer() const { return _gpu.get(); }

        void FillSampledImages(uint32_t binding, uint32_t count, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void WriteCombinedImageSampler(uint32_t binding, uint32_t arrayElement, VkImageView view, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void WriteSampledImage(uint32_t binding, uint32_t arrayElement, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void WriteSampler(uint32_t binding, uint32_t arrayElement, VkSampler sampler);
        void WriteBuffer(uint32_t binding, uint32_t arrayElement, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type);
        void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
        StaleDescriptorBuffers RecordSync(VkCommandBuffer cmd);
        VkDeviceAddress GetDeviceAddress() const { return _gpu->GetDeviceAddress(); }
    private:
        void WriteDescriptor(uint32_t binding, uint32_t arrayElement, size_t descriptorSize, const VkDescriptorGetInfoEXT& getInfo);
    private:
        VkDescriptorSetLayout _layout;
        size_t _layoutSizeInBytes = 0;

		bool _isDirty = false;
        std::shared_ptr<Buffer> _mapped;
        std::shared_ptr<Buffer> _gpu;
        Vk::BufferConfig _gpuConfig;
        Vk::BufferConfig _mappedConfig;

        size_t _combinedImageSamplerSize = 0;
        size_t _sampledImageSize = 0;
        size_t _samplerSize = 0;
        size_t _storageBufferSize = 0;
        size_t _uniformBufferSize = 0;

        std::mutex _bufferMutex;
    };
}