#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_PREFIX_SUM_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_PREFIX_SUM_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterPrefixSumPC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif