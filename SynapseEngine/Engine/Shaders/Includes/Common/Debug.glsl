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