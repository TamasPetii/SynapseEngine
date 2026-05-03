#ifndef SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct HizLinearizeDepthPC {
    uint64_t frameGlobalContextBufferAddr;
    vec2 outImageSize;
    uint activeCameraEntity;
};

#endif