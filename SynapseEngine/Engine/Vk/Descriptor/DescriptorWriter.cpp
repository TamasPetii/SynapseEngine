#include "DescriptorWriter.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn::Vk {

    DescriptorWriter& DescriptorWriter::AddCombinedImageSampler(uint32_t binding, uint32_t arrayElement, VkImageView view, VkSampler sampler, VkImageLayout layout) {
        VkDescriptorImageInfo info{ sampler, view, layout };
        _imageWrites.push_back({ binding, arrayElement, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, info });
        return *this;
    }

    DescriptorWriter& DescriptorWriter::AddSampledImage(uint32_t binding, uint32_t arrayElement, VkImageView view, VkImageLayout layout) {
        VkDescriptorImageInfo info{ VK_NULL_HANDLE, view, layout };
        _imageWrites.push_back({ binding, arrayElement, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, info });
        return *this;
    }

    DescriptorWriter& DescriptorWriter::AddSampler(uint32_t binding, uint32_t arrayElement, VkSampler sampler) {
        VkDescriptorImageInfo info{ sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
        _imageWrites.push_back({ binding, arrayElement, VK_DESCRIPTOR_TYPE_SAMPLER, info });
        return *this;
    }

    DescriptorWriter& DescriptorWriter::AddBuffer(uint32_t binding, uint32_t arrayElement, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type) {
        VkDescriptorBufferInfo info{ buffer, offset, range };
        _bufferWrites.push_back({ binding, arrayElement, type, info });
        return *this;
    }

    void DescriptorWriter::UpdateSet(VkDescriptorSet set) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(_imageWrites.size() + _bufferWrites.size());

        for (const auto& imgData : _imageWrites) {
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = set;
            write.dstBinding = imgData.binding;
            write.dstArrayElement = imgData.arrayElement;
            write.descriptorCount = 1;
            write.descriptorType = imgData.type;
            write.pImageInfo = &imgData.info;
            writes.push_back(write);
        }

        for (const auto& bufData : _bufferWrites) {
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstSet = set;
            write.dstBinding = bufData.binding;
            write.descriptorCount = 1;
            write.descriptorType = bufData.type;
            write.pBufferInfo = &bufData.info;
			write.dstArrayElement = bufData.arrayElement;
            writes.push_back(write);
        }

        if (!writes.empty()) {
            vkUpdateDescriptorSets(device->Handle(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }

        _imageWrites.clear();
        _bufferWrites.clear();
    }
}