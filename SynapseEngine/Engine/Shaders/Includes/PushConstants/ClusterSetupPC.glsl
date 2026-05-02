#ifndef SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_SETUP_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_CLUSTER_SETUP_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct ClusterSetupPC {
    uint64_t tileGridListAddr;
    uint64_t clusterCountAddr;
    uint64_t clusterListAddr;   
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint activeCameraEntity;
    uint tileSize;
    uint tileCountX;
    uint tileCountY;
    float screenWidth;
    float screenHeight;
    float hizMipLevel;
    float sliceScaleFactor;
};
#endif