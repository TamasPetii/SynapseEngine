#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Engine/Vk/Synchronization/Fence.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/SynMacro.h"

namespace Syn::Vk {

    CommandPool::CommandPool(ThreadSafeQueue* queue, VkCommandPoolCreateFlags flags)
        : _queue(queue) 
    {
		SYN_ASSERT(queue, "Queue pointer cannot be null!");

        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.queueFamilyIndex = _queue->GetFamilyIndex();
        poolInfo.flags = flags;

        SYN_VK_ASSERT_MSG(vkCreateCommandPool(device->Handle(), &poolInfo, nullptr, &_handle), "Failed to create Command Pool");
    }

    CommandPool::~CommandPool() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::GetVkContext()->GetDevice();
            vkDestroyCommandPool(device->Handle(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

    std::vector<std::unique_ptr<CommandBuffer>> CommandPool::AllocateBuffers(uint32_t count, VkCommandBufferLevel level) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = _handle;
        allocInfo.level = level;
        allocInfo.commandBufferCount = count;

        std::vector<VkCommandBuffer> handles(count);
        SYN_VK_ASSERT_MSG(vkAllocateCommandBuffers(device->Handle(), &allocInfo, handles.data()), "Failed to allocate Command Buffers");

        std::vector<std::unique_ptr<CommandBuffer>> buffers;
        buffers.reserve(count);

        for (auto handle : handles) {
            buffers.push_back(std::make_unique<CommandBuffer>(handle, _handle));
        }

        return buffers;
    }

    std::unique_ptr<CommandBuffer> CommandPool::AllocateBuffer(VkCommandBufferLevel level) {
        return std::move(AllocateBuffers(1, level)[0]);
    }

    void CommandPool::SubmitImmediate(const std::function<void(VkCommandBuffer cmd)>& function) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = _handle;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device->Handle(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        function(commandBuffer);

        vkEndCommandBuffer(commandBuffer);

        VkCommandBufferSubmitInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdInfo.commandBuffer = commandBuffer;

        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdInfo;

        Fence fence;
        _queue->Submit(&submitInfo, fence.Handle());
        fence.Wait();

        vkFreeCommandBuffers(device->Handle(), _handle, 1, &commandBuffer);
    }
}