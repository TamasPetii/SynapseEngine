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

#include "CommandBuffer.h"

namespace Syn::Vk {

    CommandBuffer::CommandBuffer(VkCommandBuffer handle, VkCommandPool pool)
        : _handle(handle), _ownerPool(pool) {
    }

    CommandBuffer::~CommandBuffer() {
    }

    void CommandBuffer::Begin(VkCommandBufferUsageFlags flags) {
        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = flags;
        SYN_VK_ASSERT_MSG(vkBeginCommandBuffer(_handle, &beginInfo), "Failed to begin Command Buffer");
    }

    void CommandBuffer::End() {
        SYN_VK_ASSERT_MSG(vkEndCommandBuffer(_handle), "Failed to end Command Buffer");
    }

    void CommandBuffer::Reset(VkCommandBufferResetFlags flags) {
        SYN_VK_ASSERT_MSG(vkResetCommandBuffer(_handle, flags), "Failed to reset Command Buffer");
    }
}