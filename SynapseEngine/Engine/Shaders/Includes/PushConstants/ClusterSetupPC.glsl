#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_SETUP_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_SETUP_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterSetupPC {
    uint64_t frameGlobalContextBufferAddr;
};
#endif