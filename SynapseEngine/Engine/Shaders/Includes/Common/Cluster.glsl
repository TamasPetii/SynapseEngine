#ifndef SYN_INCLUDES_COMMON_FORWARD_PLUS_GLSL
#define SYN_INCLUDES_COMMON_FORWARD_PLUS_GLSL

#include "../Core.glsl"
#include "IndirectCommand.glsl"

struct TileData {
    float minZ;
    float maxZ;
    uint sliceCount;
    uint clusterBaseOffset;
};

struct ClusterData {
    uint tileIndex;
    uint sliceIndex; 
    float minZ;
    float maxZ;
    
    uint pointLightCount;
    uint pointLightOffset;
    uint spotLightCount;
    uint spotLightOffset;
};

layout(buffer_reference, std430) restrict buffer TileGridBuffer { 
    TileData data[]; 
};

layout(buffer_reference, std430) restrict buffer ClusterListBuffer { 
    ClusterData data[]; 
};

layout(buffer_reference, std430) restrict buffer LightIndexListBuffer { 
    uint data[]; 
};


#define GET_TILE_DATA(addr, idx)         TileGridBuffer(addr).data[idx]
#define GET_CLUSTER_DATA(addr, idx)      ClusterListBuffer(addr).data[idx]
#define GET_LIGHT_INDEX(addr, idx)       LightIndexListBuffer(addr).data[idx]
#define GET_CLUSTER_COUNT(addr)          GET_VK_DISPATCH_CMD(addr).groupCountX

#endif