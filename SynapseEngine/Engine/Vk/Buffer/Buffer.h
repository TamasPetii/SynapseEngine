#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {

    class BufferFactory;

    struct SYN_API BufferConfig {
        VkDeviceSize size = 0;
        VkBufferUsageFlags usage = 0;
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags allocationFlags = 0;
        bool useDeviceAddress = true;
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

        void Write(const void* data, size_t size, size_t offset = 0) {
            uint8_t* ptr = static_cast<uint8_t*>(Map());
            memcpy(ptr + offset, data, size);

            if (!_isCoherent) {
                Flush(offset, size);
            }

            Unmap();
        }
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