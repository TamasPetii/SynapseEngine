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
#include "Engine/Vk/Core/ThreadSafeQueue.h"

namespace Syn::Vk {

    class CommandBuffer;

    class SYN_API CommandPool {
    public:
        explicit CommandPool(ThreadSafeQueue* queue, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        ~CommandPool();

        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        VkCommandPool Handle() const { return _handle; }

        std::vector<std::unique_ptr<CommandBuffer>> AllocateBuffers(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        std::unique_ptr<CommandBuffer> AllocateBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        void SubmitImmediate(const std::function<void(VkCommandBuffer cmd)>& function);
    private:
        VkCommandPool _handle = VK_NULL_HANDLE;
        ThreadSafeQueue* _queue;
    };
}