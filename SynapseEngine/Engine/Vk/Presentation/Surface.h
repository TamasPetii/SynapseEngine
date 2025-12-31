#pragma once
#include "../VkCommon.h"
#include "../Core/Instance.h"

namespace Syn::Vk {
    class SYN_API Surface {
    public:
        Surface(const Instance& instance, std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback);
        ~Surface();

        VkSurfaceKHR Handle() const { return _handle; }
    private:
        const Instance& _instance;
        VkSurfaceKHR _handle = VK_NULL_HANDLE;
    };
}