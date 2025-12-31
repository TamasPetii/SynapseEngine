#pragma once
#include "../VkCommon.h"
#include "PhysicalDevice.h"
#include "ThreadSafeQueue.h"

namespace Syn::Vk {
    class SYN_API Device {
    public:
        Device(VkInstance instance, const PhysicalDevice& physicalDevice, std::span<const char*> requiredExtensions);
        ~Device();

        VkDevice Handle() const { return _handle; }
        VmaAllocator GetAllocator() const { return _allocator; }
        ThreadSafeQueue& GetGraphicsQueue() const { return *_graphicsQueue; }
        ThreadSafeQueue& GetComputeQueue() const { return *_computeQueue; }
        ThreadSafeQueue& GetTransferQueue() const { return *_transferQueue; }
    private:
        void InitVMA(VkInstance instance, const PhysicalDevice& physicalDevice);
    private:
        VkDevice _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;
        std::shared_ptr<ThreadSafeQueue> _graphicsQueue;
        std::shared_ptr<ThreadSafeQueue> _computeQueue;
        std::shared_ptr<ThreadSafeQueue> _transferQueue;
    };
}