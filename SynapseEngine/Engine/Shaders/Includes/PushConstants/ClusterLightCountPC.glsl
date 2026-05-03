#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_LIGHT_COUNT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_LIGHT_COUNT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterLightCountPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif