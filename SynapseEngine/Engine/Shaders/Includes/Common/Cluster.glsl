#ifndef SYN_INCLUDES_COMMON_FORWARD_PLUS_GLSL
#define SYN_INCLUDES_COMMON_FORWARD_PLUS_GLSL

#include "../Core.glsl"
#include "IndirectCommand.glsl"

struct ActiveClusterData {
    uint tileIndex;
    uint sliceIndex; 
    float minZ;
    float maxZ;
};

layout(buffer_reference, std430) restrict buffer ActiveClusterListBuffer { 
    ActiveClusterData data[]; 
};

#define GET_ACTIVE_CLUSTER(addr, idx)    ActiveClusterListBuffer(addr).data[idx]
#define GET_CLUSTER_COUNT(addr)          GET_VK_DISPATCH_CMD(addr).groupCountX

#endif