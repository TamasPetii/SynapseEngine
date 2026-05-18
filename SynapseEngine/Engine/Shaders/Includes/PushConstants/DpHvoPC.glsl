#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DPHVO_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DPHVO_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DpHvoPC {
    uint64_t frameGlobalContextBufferAddr;
    float aoRadius;
    float aoIntensity;
    float maxOcclusionDistance;
    float bias;
    uint sampleCount;
};

#endif