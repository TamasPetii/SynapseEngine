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
        ThreadSafeQueue* GetGraphicsQueue() const { return _graphicsQueue.get(); }
        ThreadSafeQueue* GetComputeQueue() const { return _computeQueue.get(); }
        ThreadSafeQueue* GetTransferQueue() const { return _transferQueue.get(); }
        ThreadSafeQueue* GetVideoDecodeQueue() const { return _videoDecodeQueue.get(); }
		void WaitIdle() const;
        void SetDebugName(VkObjectType objectType, uint64_t objectHandle, const char* name) const;
    private:
        void InitVMA(VkInstance instance, const PhysicalDevice& physicalDevice);
    private:
        VkDevice _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;
        std::shared_ptr<ThreadSafeQueue> _graphicsQueue;
        std::shared_ptr<ThreadSafeQueue> _computeQueue;
        std::shared_ptr<ThreadSafeQueue> _transferQueue;
        std::shared_ptr<ThreadSafeQueue> _videoDecodeQueue;
    };
}