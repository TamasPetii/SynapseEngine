#include "Semaphore.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/SynMacro.h"

namespace Syn::Vk {

    Semaphore::Semaphore() = default;

    Semaphore::~Semaphore() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::GetVkContext()->GetDevice();
            vkDestroySemaphore(device->Handle(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

    Semaphore::Semaphore(Semaphore&& other) noexcept {
        _handle = other._handle;
        other._handle = VK_NULL_HANDLE;
    }

    Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
        if (this != &other) {
            if (_handle != VK_NULL_HANDLE) {
                auto device = ServiceLocator::GetVkContext()->GetDevice();
                vkDestroySemaphore(device->Handle(), _handle, nullptr);
            }

            _handle = other._handle;
            other._handle = VK_NULL_HANDLE;
        }
        return *this;
    }
}