#pragma once
#include "Buffer.h"

namespace Syn::Vk {

    class SYN_API BufferFactory {
    public:
        static void Allocate(Buffer* buffer);
        static std::unique_ptr<Buffer> CreateGpu(VkDeviceSize size,VkBufferUsageFlags usage);
        static std::unique_ptr<Buffer> CreatePersistent(VkDeviceSize size,VkBufferUsageFlags usage);
        static std::unique_ptr<Buffer> CreateStaging(VkDeviceSize size);
        static std::unique_ptr<Buffer> Create(const BufferConfig& config);
    };
}