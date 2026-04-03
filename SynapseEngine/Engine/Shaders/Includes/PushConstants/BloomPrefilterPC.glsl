#ifndef SYN_INCLUDES_PUSH_CONSTANTS_BLOOM_PREFILTER_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_BLOOM_PREFILTER_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct BloomPrefilterPC {
    vec2 texelSize;
	float threshold;
	float knee;
};

#endif