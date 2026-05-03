#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_LIGHT_WRITE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_LIGHT_WRITE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterLightWritePC {
    uint64_t frameGlobalContextBufferAddr;
};

#endif