#ifndef SYN_INCLUDES_PUSH_CONSTANTS_BLOOM_UP_SAMPLE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_BLOOM_UP_SAMPLE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct BloomUpSamplePC {
    vec2 texelSize;
	float filterRadius;
};

#endif