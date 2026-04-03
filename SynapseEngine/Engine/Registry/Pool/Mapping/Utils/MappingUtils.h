#pragma once
#include "Engine/SynApi.h"
#include <cstdint>
#include <bit>

namespace Syn
{
    constexpr uint32_t ConstLog2(uint32_t n) {
        return n == 0 ? 0 : std::bit_width(n) - 1;
    }
}