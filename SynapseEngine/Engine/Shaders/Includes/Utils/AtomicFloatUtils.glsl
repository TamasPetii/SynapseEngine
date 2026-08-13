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

#ifndef SYN_INCLUDES_UTILS_ATOMIC_FLOAT_UTILS_GLSL
#define SYN_INCLUDES_UTILS_ATOMIC_FLOAT_UTILS_GLSL

uint floatToSortableUint(float f) {
    uint u = floatBitsToUint(f);
    uint mask = uint(-int(u >> 31)) | 0x80000000u;
    return u ^ mask;
}

float sortableUintToFloat(uint u) {
    uint mask = ((u >> 31) - 1u) | 0x80000000u;
    return uintBitsToFloat(u ^ mask);
}

#endif