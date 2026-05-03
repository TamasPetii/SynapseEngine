#ifndef SYN_INCLUDES_PUSH_CONSTANTS_SPOT_LIGHT_CULLING_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_SPOT_LIGHT_CULLING_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct SpotLightCullingPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif