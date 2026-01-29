#pragma once
#include "Engine/SynApi.h"
#include "Semaphore.h"
#include <volk.h>
#include <cstdint>

namespace Syn::Vk {
    class SYN_API BinarySemaphore : public Semaphore {
    public:
        BinarySemaphore();
    };
}