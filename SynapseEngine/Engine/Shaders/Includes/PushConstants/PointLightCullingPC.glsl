#ifndef SYN_INCLUDES_PUSH_CONSTANTS_POINT_LIGHT_CULLING_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_POINT_LIGHT_CULLING_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct PointLightCullingPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif