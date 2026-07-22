#ifndef SYN_INCLUDES_PC_PHYSICS_DEBUG_PASS_GLSL
#define SYN_INCLUDES_PC_PHYSICS_DEBUG_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct PhysicsDebugPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t joltDebugVertexBufferAddr;
    uint64_t joltDebugIndexBufferAddr;
    uint64_t joltDebugInstanceBufferAddr;
};

#endif