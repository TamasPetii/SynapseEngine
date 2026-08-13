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
#include <cstring>
#include <string>
#include "../VkCommon.h"

namespace Syn::Vk {

    class BufferFactory;

    struct SYN_API BufferConfig {
        std::string debugName = "";
        VkDeviceSize size = 0;
        VkBufferUsageFlags usage = 0;
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags allocationFlags = 0;
        bool useDeviceAddress = true;
        const void* pNextExtension = nullptr;
    };

    class SYN_API Buffer {
    public:
        explicit Buffer(const BufferConfig& config);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator=(Buffer&&) = delete;

        VkBuffer Handle() const { return _handle; }
        VkDeviceAddress GetDeviceAddress() const { return _deviceAddress; }
        VkDeviceSize GetSize() const { return _config.size; }
        const BufferConfig& GetConfig() const { return _config; }

        bool IsCoherent() const { return _isCoherent; }
        bool IsPersistent() const { return _persistentMappedData != nullptr; }

        void* Map();
        void Unmap();

        void Flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
        void Write(const void* data, size_t size, size_t offset = 0);
    private:
        BufferConfig _config;

        VmaAllocator _allocator = VK_NULL_HANDLE;
        VkBuffer _handle = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        VkDeviceAddress _deviceAddress = 0;
        VkDeviceSize _size = 0;

        void* _persistentMappedData = nullptr;
        bool _isCoherent = false;
        bool _isMapped = false;

        friend class BufferFactory;
    };
}