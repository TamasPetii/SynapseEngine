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

#ifndef SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct InfiniteGridPC {
    uint64_t frameGlobalContextBufferAddr;
    uint planeFlags;     // Bit 0: XZ, Bit 1: XY, Bit 2: YZ
    uint axisFlags;      // Bit 0: X,  Bit 1: Y,  Bit 2: Z
    float gridScale;
    float fadeDistance;
    float gridThickness;
    float axisThickness;
    vec4 gridColor;
    vec4 axisXColor;
    vec4 axisYColor;
    vec4 axisZColor;
};

#endif