#ifndef SYN_INCLUDES_PC_POINT_LIGHT_SHADOW_CULLING_PASS_GLSL
#define SYN_INCLUDES_PC_POINT_LIGHT_SHADOW_CULLING_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct PointLightShadowCullingPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif