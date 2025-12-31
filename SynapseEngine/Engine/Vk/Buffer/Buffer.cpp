#include "Buffer.h"
#include "BufferFactory.h"
#include "Engine/SynMacro.h"

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
            void* data;
            SYN_VK_ASSERT_MSG(vmaMapMemory(_allocator, _allocation, &data), "Failed to map buffer memory");
            return data;
        }

        void* data;
        SYN_VK_ASSERT_MSG(vmaMapMemory(_allocator, _allocation, &data), "Failed to map buffer memory");
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
}