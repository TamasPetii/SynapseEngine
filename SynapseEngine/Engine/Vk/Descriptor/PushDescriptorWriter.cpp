#include "PushDescriptorWriter.h"

namespace Syn::Vk {

    PushDescriptorWriter& PushDescriptorWriter::AddCombinedImageSampler(uint32_t binding, VkImageView view, VkSampler sampler, VkImageLayout layout)
    {
        VkDescriptorImageInfo info{};
        info.imageView = view;
        info.sampler = sampler;
        info.imageLayout = layout;

        _imageWrites.push_back({ binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, info });
        return *this;
    }

    PushDescriptorWriter& PushDescriptorWriter::AddSampledImage(uint32_t binding, VkImageView view, VkImageLayout layout)
    {
        VkDescriptorImageInfo info{};
        info.imageView = view;
        info.imageLayout = layout;

        _imageWrites.push_back({ binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, info });
        return *this;
    }

    PushDescriptorWriter& PushDescriptorWriter::AddSampler(uint32_t binding, VkSampler sampler)
    {
        VkDescriptorImageInfo info{};
        info.sampler = sampler;

        _imageWrites.push_back({ binding, VK_DESCRIPTOR_TYPE_SAMPLER, info });
        return *this;
    }

    PushDescriptorWriter& PushDescriptorWriter::AddBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type)
    {
        VkDescriptorBufferInfo info{};
        info.buffer = buffer;
        info.offset = offset;
        info.range = range;

        _bufferWrites.push_back({ binding, type, info });
        return *this;
    }

    void PushDescriptorWriter::Push(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex, VkPipelineBindPoint bindPoint)
    {
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(_imageWrites.size() + _bufferWrites.size());

        for (const auto& imgData : _imageWrites) {
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = 0;
            write.dstBinding = imgData.binding;
            write.descriptorCount = 1;
            write.descriptorType = imgData.type;
            write.pImageInfo = &imgData.info;
            writes.push_back(write);
        }

        for (const auto& bufData : _bufferWrites) {
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = 0;
            write.dstBinding = bufData.binding;
            write.descriptorCount = 1;
            write.descriptorType = bufData.type;
            write.pBufferInfo = &bufData.info;
            writes.push_back(write);
        }

        if (!writes.empty()) {
            vkCmdPushDescriptorSetKHR(
                cmd,
                bindPoint,
                pipelineLayout,
                setIndex,
                static_cast<uint32_t>(writes.size()),
                writes.data()
            );
        }

        _imageWrites.clear();
        _bufferWrites.clear();
    }
}