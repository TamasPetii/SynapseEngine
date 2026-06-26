#ifndef SYN_INCLUDES_PUSH_CONSTANTS_SSAO_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_SSAO_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct SsaoPC {
    uint64_t frameGlobalContextBufferAddr;
    float aoRadius;
    float aoIntensity;
    float maxOcclusionDistance;
    float bias;
    uint sampleCount;
    float noiseTextureWidth;
    float noiseTextureHeight;
};

#endif