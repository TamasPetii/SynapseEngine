#pragma once
#include "../VkCommon.h"
#include <vector>

namespace Syn::Vk {

    class SYN_API DescriptorWriter {
    public:
        DescriptorWriter() = default;
        ~DescriptorWriter() = default;

        DescriptorWriter& AddCombinedImageSampler(uint32_t binding, uint32_t arrayElement, VkImageView view, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        DescriptorWriter& AddSampledImage(uint32_t binding, uint32_t arrayElement, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        DescriptorWriter& AddSampler(uint32_t binding, uint32_t arrayElement, VkSampler sampler);
        DescriptorWriter& AddBuffer(uint32_t binding, uint32_t arrayElement, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        void UpdateSet(VkDescriptorSet set);
    private:
        struct ImageWriteData {
            uint32_t binding;
            uint32_t arrayElement;
            VkDescriptorType type;
            VkDescriptorImageInfo info;
        };

        struct BufferWriteData {
            uint32_t binding;
            uint32_t arrayElement;
            VkDescriptorType type;
            VkDescriptorBufferInfo info;
        };

        std::vector<ImageWriteData> _imageWrites;
        std::vector<BufferWriteData> _bufferWrites;
    };
}