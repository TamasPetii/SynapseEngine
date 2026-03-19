#include "DescriptorBuffer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Buffer/BufferFactory.h"

namespace Syn::Vk {

    DescriptorBuffer::DescriptorBuffer(VkDescriptorSetLayout layout)
        : _layout(layout)
    {
        auto context = ServiceLocator::GetVkContext();
        auto device = context->GetDevice();
        auto physicalDevice = context->GetPhysicalDevice();

        const auto& props = physicalDevice->GetDescriptorBufferProperties();
        _combinedImageSamplerSize = props.combinedImageSamplerDescriptorSize;
        _sampledImageSize = props.sampledImageDescriptorSize;
        _samplerSize = props.samplerDescriptorSize;
        _storageBufferSize = props.storageBufferDescriptorSize;
        _uniformBufferSize = props.uniformBufferDescriptorSize;

        VkDeviceSize layoutSizeInBytes = 0;
        vkGetDescriptorSetLayoutSizeEXT(device->Handle(), _layout, &layoutSizeInBytes);

        VkBufferUsageFlags usage =
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        _buffer = BufferFactory::CreatePersistent(layoutSizeInBytes, usage);
    }

    void DescriptorBuffer::FillSampledImages(uint32_t binding, uint32_t count, VkImageView view, VkImageLayout layout)
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkDescriptorImageInfo imageInfo{ VK_NULL_HANDLE, view, layout };

        VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        getInfo.data.pSampledImage = &imageInfo;

        VkDeviceSize bindingOffset;
        vkGetDescriptorSetLayoutBindingOffsetEXT(device->Handle(), _layout, binding, &bindingOffset);

        void* mappedData = _buffer->Map();
        char* targetBaseAddress = static_cast<char*>(mappedData) + bindingOffset;

        std::vector<char> descriptorPayload(_sampledImageSize);
        vkGetDescriptorEXT(device->Handle(), &getInfo, _sampledImageSize, descriptorPayload.data());

        for (uint32_t i = 0; i < count; ++i) {
            std::memcpy(targetBaseAddress + (i * _sampledImageSize), descriptorPayload.data(), _sampledImageSize);
        }
    }

    void DescriptorBuffer::WriteDescriptor(uint32_t binding, uint32_t arrayElement, size_t descriptorSize, const VkDescriptorGetInfoEXT& getInfo)
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkDeviceSize bindingOffset;
        vkGetDescriptorSetLayoutBindingOffsetEXT(device->Handle(), _layout, binding, &bindingOffset);

        void* mappedData = _buffer->Map();
        char* targetAddress = static_cast<char*>(mappedData) + bindingOffset + (arrayElement * descriptorSize);

        vkGetDescriptorEXT(device->Handle(), &getInfo, descriptorSize, targetAddress);
    }

    void DescriptorBuffer::WriteCombinedImageSampler(uint32_t binding, uint32_t arrayElement, VkImageView view, VkSampler sampler, VkImageLayout layout)
    {
        VkDescriptorImageInfo imageInfo{ sampler, view, layout };

        VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        getInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        getInfo.data.pCombinedImageSampler = &imageInfo;

        WriteDescriptor(binding, arrayElement, _combinedImageSamplerSize, getInfo);
    }

    void DescriptorBuffer::WriteSampledImage(uint32_t binding, uint32_t arrayElement, VkImageView view, VkImageLayout layout)
    {
        VkDescriptorImageInfo imageInfo{ VK_NULL_HANDLE, view, layout };

        VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        getInfo.data.pSampledImage = &imageInfo;

        WriteDescriptor(binding, arrayElement, _sampledImageSize, getInfo);
    }

    void DescriptorBuffer::WriteSampler(uint32_t binding, uint32_t arrayElement, VkSampler sampler)
    {
        VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLER;
        getInfo.data.pSampler = &sampler;

        WriteDescriptor(binding, arrayElement, _samplerSize, getInfo);
    }

    void DescriptorBuffer::WriteBuffer(uint32_t binding, uint32_t arrayElement, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type)
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkBufferDeviceAddressInfo bdaInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
        bdaInfo.buffer = buffer;
        VkDeviceAddress address = vkGetBufferDeviceAddress(device->Handle(), &bdaInfo);

        VkDescriptorAddressInfoEXT addrInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
        addrInfo.address = address + offset;
        addrInfo.range = range;
        addrInfo.format = VK_FORMAT_UNDEFINED;

        VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        getInfo.type = type;

        size_t descriptorSize = 0;

        if (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            getInfo.data.pStorageBuffer = &addrInfo;
            descriptorSize = _storageBufferSize;
        }
        else if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            getInfo.data.pUniformBuffer = &addrInfo;
            descriptorSize = _uniformBufferSize;
        }

        WriteDescriptor(binding, arrayElement, descriptorSize, getInfo);
    }

    void DescriptorBuffer::Bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t setIndex, VkPipelineBindPoint bindPoint)
    {
        VkDescriptorBufferBindingInfoEXT bindingInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT };
        bindingInfo.address = _buffer->GetDeviceAddress();
        bindingInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        vkCmdBindDescriptorBuffersEXT(cmd, 1, &bindingInfo);

        uint32_t bufferIndex = 0;
        VkDeviceSize offset = 0;

        vkCmdSetDescriptorBufferOffsetsEXT(cmd, bindPoint, pipelineLayout, setIndex, 1, &bufferIndex, &offset);
    }
}