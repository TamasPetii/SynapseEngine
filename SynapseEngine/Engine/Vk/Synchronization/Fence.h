#pragma once
#include "Engine/SynApi.h"
#include <volk.h>

namespace Syn::Vk {

    class SYN_API Fence {
    public:
        explicit Fence(bool signaled = false);
        ~Fence();

        Fence(const Fence&) = delete;
        Fence& operator=(const Fence&) = delete;
        Fence(Fence&&) = delete;
        Fence& operator=(Fence&&) = delete;

        VkFence Handle() const { return _handle; }

        void Reset();
        void Wait(uint64_t timeout = UINT64_MAX);
        bool IsSignaled() const;
    private:
        VkFence _handle = VK_NULL_HANDLE;
    };
}