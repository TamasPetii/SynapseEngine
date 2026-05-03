#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_DIRECTION_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_DIRECTION_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DeferredDirectionLightPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif