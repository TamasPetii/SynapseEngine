#pragma once
#include "../VkCommon.h"
#include "Semaphore.h"

namespace Syn::Vk {
    class SYN_API TimelineSemaphore : public Semaphore {
    public:
        explicit TimelineSemaphore(uint64_t initialValue = 0);

        void Signal(uint64_t value);
        void Wait(uint64_t value, uint64_t timeout = UINT64_MAX);
        uint64_t GetValue() const;
    };
}