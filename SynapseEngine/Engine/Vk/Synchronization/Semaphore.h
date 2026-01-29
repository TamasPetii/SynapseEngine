#pragma once
#include "Engine/SynApi.h"
#include <volk.h>
#include <cstdint>

namespace Syn::Vk {
    class SYN_API Semaphore {
    public:
        virtual ~Semaphore();

        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;
        Semaphore(Semaphore&& other) noexcept;
        Semaphore& operator=(Semaphore&& other) noexcept;

        VkSemaphore Handle() const { return _handle; }
    protected:
        Semaphore();
        VkSemaphore _handle = VK_NULL_HANDLE;
    };
}