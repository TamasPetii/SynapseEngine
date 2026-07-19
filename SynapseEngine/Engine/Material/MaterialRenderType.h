#pragma once
#include <cstdint>

namespace Syn
{
    enum MaterialRenderType : uint32_t {
        Opaque1Sided = 0,
        Opaque2Sided = 1,
        AlphaTestedOpaque1Sided = 2,
        AlphaTestedOpaque2Sided = 3,
        Transparent1Sided = 4,
        Transparent2Sided = 5,
        AlphaTestedTransparent1Sided = 6,
        AlphaTestedTransparent2Sided = 7,
        Count = 8
    };
}