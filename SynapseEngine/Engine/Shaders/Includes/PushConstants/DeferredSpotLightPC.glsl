#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_SPOT_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_SPOT_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DeferredSpotLightPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t vertexPositionBufferAddr;
    uint64_t indexBufferAddr;
};

#endif