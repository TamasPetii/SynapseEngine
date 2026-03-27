#pragma once
#include <cstdint>

namespace Syn
{
    enum MaterialRenderType : uint32_t {
        Opaque1Sided = 0,
        Opaque2Sided = 1,
        Transparent1Sided = 2,
        Transparent2Sided = 3,
        Count = 4
    };
}