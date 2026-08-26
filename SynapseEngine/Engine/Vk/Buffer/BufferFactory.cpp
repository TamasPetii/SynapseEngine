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

#include "BufferFactory.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "BufferUtils.h"

namespace Syn::Vk {

    void BufferFactory::Allocate(Buffer* buffer) {
        auto context = ServiceLocator::Get<Vk::Context>();
        auto device = context->GetDevice();
        VmaAllocator allocator = device->GetAllocator();

        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = buffer->_config.size;
        bufferInfo.usage = buffer->_config.usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.pNext = buffer->_config.pNextExtension;

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
        
        if (!buffer->GetConfig().debugName.empty()) {
            device->SetDebugName(
                VK_OBJECT_TYPE_BUFFER,
                (uint64_t)buffer->Handle(),
                buffer->GetConfig().debugName.c_str()
            );
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
        config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        config.useDeviceAddress = false;

        return std::make_unique<Buffer>(config);
    }

    std::unique_ptr<Buffer> BufferFactory::CreateVideoBitstream(VkDeviceSize size, const void* pNextExtension) {
        BufferConfig config;
        config.size = size;
        config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        config.useDeviceAddress = false;
        config.pNextExtension = pNextExtension;

        return std::make_unique<Buffer>(config);
    }
}