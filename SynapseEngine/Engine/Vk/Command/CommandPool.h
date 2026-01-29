#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include <volk.h>
#include <vector>
#include <memory>
#include <functional>

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