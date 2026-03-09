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