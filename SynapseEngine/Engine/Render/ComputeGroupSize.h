#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
    struct SYN_API ComputeGroupSize
    {
        static constexpr uint32_t Image8D = 8;
        static constexpr uint32_t Image16D = 16;
        static constexpr uint32_t Image32D = 32;

        static constexpr uint32_t Buffer32D = 32;
        static constexpr uint32_t Buffer64D = 64;
        static constexpr uint32_t Buffer128D = 128;
        static constexpr uint32_t Buffer256D = 256;

        static constexpr uint32_t CalculateDispatchCount(uint32_t elementCount, uint32_t groupSize)
        {
            return (elementCount + groupSize - 1) / groupSize;
        }
    };
}