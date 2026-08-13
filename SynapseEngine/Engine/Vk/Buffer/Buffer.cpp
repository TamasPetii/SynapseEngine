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

#include "Buffer.h"
#include "BufferFactory.h"

namespace Syn::Vk {

    Buffer::Buffer(const BufferConfig& config)
        : _config(config)
    {
        BufferFactory::Allocate(this);
    }

    Buffer::~Buffer() {
        if (_handle != VK_NULL_HANDLE && _allocator != VK_NULL_HANDLE) {
            if (_isMapped && _persistentMappedData == nullptr) {
                vmaUnmapMemory(_allocator, _allocation);
            }
            vmaDestroyBuffer(_allocator, _handle, _allocation);
        }
    }

    void* Buffer::Map() {
        if (_persistentMappedData != nullptr) {
            return _persistentMappedData;
        }

        if (_isMapped) {
            void* data = nullptr;

            if (vmaMapMemory(_allocator, _allocation, &data) != VK_SUCCESS) 
                return nullptr;

            return data;
        }

        void* data = nullptr;
        VkResult res = vmaMapMemory(_allocator, _allocation, &data);

        if (res != VK_SUCCESS) {
            return nullptr;
        }

        _isMapped = true;
        return data;
    }

    void Buffer::Unmap() {
        if (_persistentMappedData != nullptr) 
            return;

        if (_isMapped) {
            vmaUnmapMemory(_allocator, _allocation);
            _isMapped = false;
        }
    }

    void Buffer::Flush(VkDeviceSize offset, VkDeviceSize size) {
        if (_isCoherent) 
            return;

        vmaFlushAllocation(_allocator, _allocation, offset, size);
    }

    void Buffer::Write(const void* data, size_t size, size_t offset) {
        uint8_t* ptr = static_cast<uint8_t*>(Map());
        memcpy(ptr + offset, data, size);

        if (!_isCoherent) {
            Flush(offset, size);
        }

        Unmap();
    }
}