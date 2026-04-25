#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_SETUP_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_SETUP_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterSetupPC {
    uint64_t activeClusterCountAddr;
    uint64_t activeClusterListAddr;   
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint activeCameraEntity;
    uint tileCountX;
    uint tileCountY;
    float screenWidth;
    float screenHeight;
    uint tileSize;
    float hizMipLevel;
};
#endif