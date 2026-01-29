#pragma once
#include "Engine/SynApi.h"
#include <volk.h>

namespace Syn::Vk {

    class CommandPool;

    class SYN_API CommandBuffer {
    public:
        CommandBuffer(VkCommandBuffer handle, VkCommandPool pool);
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&&) = default;
        CommandBuffer& operator=(CommandBuffer&&) = default;

        VkCommandBuffer Handle() const { return _handle; }

        void Begin(VkCommandBufferUsageFlags flags = 0);
        void End();
        void Reset(VkCommandBufferResetFlags flags = 0);

    private:
        VkCommandBuffer _handle = VK_NULL_HANDLE;
        VkCommandPool _ownerPool = VK_NULL_HANDLE;
    };
}