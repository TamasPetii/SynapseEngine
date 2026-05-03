#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_POINT_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_POINT_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DeferredPointLightPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t vertexPositionBufferAddr;
    uint64_t indexBufferAddr;
};

#endif