#pragma once
#include "../VkCommon.h"
#include "Semaphore.h"

namespace Syn::Vk {
    class SYN_API BinarySemaphore : public Semaphore {
    public:
        BinarySemaphore();
    };
}