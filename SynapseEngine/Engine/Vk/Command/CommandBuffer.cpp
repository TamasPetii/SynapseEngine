#include "CommandBuffer.h"
#include "Engine/SynMacro.h"

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