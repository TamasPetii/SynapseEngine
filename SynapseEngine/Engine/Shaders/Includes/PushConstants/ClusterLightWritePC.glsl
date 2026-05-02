#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_LIGHT_WRITE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_LIGHT_WRITE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterLightWritePC {
    uint64_t clusterListAddr;
    uint64_t clusterCountAddr;
    
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    
    uint64_t lightIndirectCmdAddr; 
    uint64_t visibleLightAddr;     
    uint64_t lightColliderDataAddr;
    uint64_t lightIndexListAddr; 
    
    uint activeCameraEntity;
    uint tileCountX;
    uint tileSize;
    float screenWidth;
    float screenHeight;
};

#endif