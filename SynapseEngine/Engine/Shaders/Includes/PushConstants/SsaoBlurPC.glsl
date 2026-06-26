#ifndef SYN_INCLUDES_PUSH_CONSTANTS_SSAO_BLUR_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_SSAO_BLUR_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct SsaoBlurPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif