#include "Surface.h"

namespace Syn::Vk {
    Surface::Surface(const Instance& instance, std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback)
        : _instance(instance)
    {
        createSurfaceCallback(_instance.Handle(), &_handle);
    }

    Surface::~Surface() {
        if (_handle != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(_instance.Handle(), _handle, nullptr);
        }
    }
}