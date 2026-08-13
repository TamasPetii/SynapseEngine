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

#ifndef SYN_INCLUDES_COMMON_DEBUG_PHYSICS_GLSL
#define SYN_INCLUDES_COMMON_DEBUG_PHYSICS_GLSL

#include "../Core.glsl"

struct PhysicsDebugVertex {
    vec4 position;
};

struct PhysicsDebugInstance {
    mat4 transform;
    uint color;
    uint pad0;
    uint pad1;
    uint pad2;
};

layout(buffer_reference, std430) readonly restrict buffer DebugIndexBuffer {
    uint data[];
};

layout(buffer_reference, std430) readonly restrict buffer DebugInstanceBuffer {
    PhysicsDebugInstance data[];
};

layout(buffer_reference, std430) readonly restrict buffer JoltDebugVertexBuffer { 
    PhysicsDebugVertex data[]; 
};

#define GET_PHYSICS_DEBUG_INDEX(addr, idx) DebugIndexBuffer(addr).data[idx]
#define GET_PHYSICS_DEBUG_INSTANCE(addr, idx) DebugInstanceBuffer(addr).data[idx]
#define GET_PHYSICS_DEBUG_VERTEX(addr, idx) JoltDebugVertexBuffer(addr).data[idx]

#endif