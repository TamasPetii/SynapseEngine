#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DPHVO_BLUR_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DPHVO_BLUR_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DpHvoBlurPC {
    uint64_t frameGlobalContextBufferAddr;
    vec2 blurDirection;
    float depthSharpness;
};

#endif