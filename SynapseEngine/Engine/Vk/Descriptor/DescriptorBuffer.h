#pragma once
#include "../VkCommon.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>

namespace Syn::Vk {

    class SYN_API DescriptorBuffer {
    public:
        DescriptorBuffer(VkDescriptorSetLayout layout);
        ~DescriptorBuffer() = default;

        DescriptorBuffer(const DescriptorBuffer&) = delete;
        DescriptorBuffer& operator=(const DescriptorBuffer&) = delete;

        Buffer* GetBuffer() const { return _buffer.get(); }

        void FillSampledImages(uint32_t binding, uint32_t count, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void WriteCombinedImageSampler(uint32_t binding, uint32_t arrayElement, VkImageView view, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void WriteSampledImage(uint32_t binding, uint32_t arrayElement, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void WriteSampler(uint32_t binding, uint32_t arrayElement, VkSampler sampler);
        void WriteBuffer(uint32_t binding, uint32_t arrayElement, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type);
        void Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    private:
        void WriteDescriptor(uint32_t binding, uint32_t arrayElement, size_t descriptorSize, const VkDescriptorGetInfoEXT& getInfo);
    private:
        std::unique_ptr<Buffer> _buffer;
        VkDescriptorSetLayout _layout;

        size_t _combinedImageSamplerSize = 0;
        size_t _sampledImageSize = 0;
        size_t _samplerSize = 0;
        size_t _storageBufferSize = 0;
        size_t _uniformBufferSize = 0;
    };
}