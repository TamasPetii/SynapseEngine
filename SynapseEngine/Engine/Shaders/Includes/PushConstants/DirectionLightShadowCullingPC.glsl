#ifndef SYN_INCLUDES_PC_DIRECTION_LIGHT_SHADOW_CULLING_PASS_GLSL
#define SYN_INCLUDES_PC_DIRECTION_LIGHT_SHADOW_CULLING_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct DirectionLightShadowCullingPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif