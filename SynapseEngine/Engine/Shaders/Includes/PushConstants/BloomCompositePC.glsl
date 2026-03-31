#ifndef SYN_INCLUDES_PUSH_CONSTANTS_BLOOM_COMPOSITE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_BLOOM_COMPOSITE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct BloomCompositePC {
    float exposure;
    float bloomStrength;
};

#endif