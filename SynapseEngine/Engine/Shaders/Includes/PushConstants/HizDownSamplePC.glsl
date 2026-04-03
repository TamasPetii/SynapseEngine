#ifndef SYN_INCLUDES_PUSH_CONSTANTS_HIZ_DOWN_SAMPLE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_HIZ_DOWN_SAMPLE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct HizDownSamplePC {
    vec2 inImageSize;
    vec2 outImageSize;
};

#endif