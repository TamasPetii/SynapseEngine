#ifndef SYN_INCLUDES_PC_SPOT_LIGHT_SHADOW_CULLING_PASS_GLSL
#define SYN_INCLUDES_PC_SPOT_LIGHT_SHADOW_CULLING_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct SpotLightShadowCullingPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif