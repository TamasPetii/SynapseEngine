#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>
#include <cstdint>

namespace Syn
{
    struct SYN_API StaleBuffer {
        std::shared_ptr<Vk::Buffer> buffer;
        uint32_t framesToLive;
    };
}