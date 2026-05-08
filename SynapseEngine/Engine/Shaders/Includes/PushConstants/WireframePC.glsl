#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct WireframePC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t vertexPositionBufferAddr;
    uint isSphere;
};

#endif