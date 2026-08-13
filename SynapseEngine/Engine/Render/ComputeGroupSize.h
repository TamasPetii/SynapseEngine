// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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
        static constexpr uint32_t Image64D = 64;
        static constexpr uint32_t Image128D = 128;
        static constexpr uint32_t Image256D = 256;
        static constexpr uint32_t Image512D = 512;

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