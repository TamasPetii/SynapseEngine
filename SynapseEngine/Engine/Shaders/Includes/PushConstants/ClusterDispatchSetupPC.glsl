#ifndef SYN_INCLUDES_PC_CLUSTER_DISPATCH_SETUP_GLSL
#define SYN_INCLUDES_PC_CLUSTER_DISPATCH_SETUP_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterDispatchSetupPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t dispatchArgsBufferAddr;
};

#endif