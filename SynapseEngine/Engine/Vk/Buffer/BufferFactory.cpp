#include "BufferFactory.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "BufferUtils.h"

namespace Syn::Vk {

    void BufferFactory::Allocate(Buffer* buffer) {
        auto context = ServiceLocator::GetVkContext();
        auto device = context->GetDevice();
        VmaAllocator allocator = device->GetAllocator();

        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = buffer->_config.size;
        bufferInfo.usage = buffer->_config.usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (buffer->_config.useDeviceAddress) {
            bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = buffer->_config.memoryUsage;
        allocCreateInfo.flags = buffer->_config.allocationFlags;

        VmaAllocationInfo allocResultInfo;
        SYN_VK_ASSERT_MSG(vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer->_handle, &buffer->_allocation, &allocResultInfo), "Failed to create VMA buffer");

        buffer->_allocator = allocator;
        buffer->_size = allocResultInfo.size;

        if (bufferInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfo bdaInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            bdaInfo.buffer = buffer->_handle;
            buffer->_deviceAddress = vkGetBufferDeviceAddress(device->Handle(), &bdaInfo);
        }

        VkMemoryPropertyFlags memFlags;
        vmaGetAllocationMemoryProperties(allocator, buffer->_allocation, &memFlags);
        buffer->_isCoherent = (memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

        if (allocResultInfo.pMappedData != nullptr) {
            buffer->_persistentMappedData = allocResultInfo.pMappedData;
            buffer->_isMapped = true;
        }
    }

    std::unique_ptr<Buffer> BufferFactory::Create(const BufferConfig& config) {
        return std::make_unique<Buffer>(config);
    }

    std::unique_ptr<Buffer> BufferFactory::CreateGpu(VkDeviceSize size, VkBufferUsageFlags usage) {
        BufferConfig config;
        config.size = size;
        config.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        config.useDeviceAddress = true;

        return std::make_unique<Buffer>(config);
    }

    std::unique_ptr<Buffer> BufferFactory::CreatePersistent(VkDeviceSize size, VkBufferUsageFlags usage) {
        BufferConfig config;
        config.size = size;
        config.usage = usage;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        config.useDeviceAddress = true;

        return std::make_unique<Buffer>(config);
    }

    std::unique_ptr<Buffer> BufferFactory::CreateStaging(VkDeviceSize size) {
        BufferConfig config;
        config.size = size;
        config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        config.useDeviceAddress = false;

        return std::make_unique<Buffer>(config);
    }
}