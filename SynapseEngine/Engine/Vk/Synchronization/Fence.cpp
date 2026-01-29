#include "Fence.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/SynMacro.h"

namespace Syn::Vk {

    Fence::Fence(bool signaled) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkFenceCreateInfo info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

        if (signaled) {
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        SYN_VK_ASSERT_MSG(vkCreateFence(device->Handle(), &info, nullptr, &_handle), "Failed to create Fence");
    }

    Fence::~Fence() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::GetVkContext()->GetDevice();
            vkDestroyFence(device->Handle(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

    void Fence::Reset() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();
        SYN_VK_ASSERT_MSG(vkResetFences(device->Handle(), 1, &_handle), "Failed to reset Fence");
    }

    void Fence::Wait(uint64_t timeout) {
        auto device = ServiceLocator::GetVkContext()->GetDevice();
        SYN_VK_ASSERT_MSG(vkWaitForFences(device->Handle(), 1, &_handle, VK_TRUE, timeout), "Failed to wait for Fence");
    }

    bool Fence::IsSignaled() const {
        auto device = ServiceLocator::GetVkContext()->GetDevice();
        VkResult result = vkGetFenceStatus(device->Handle(), _handle);

        if (result == VK_SUCCESS) return true;
        if (result == VK_NOT_READY) return false;

        SYN_VK_ASSERT_MSG(result, "Failed to check Fence status");
        return false;
    }
}