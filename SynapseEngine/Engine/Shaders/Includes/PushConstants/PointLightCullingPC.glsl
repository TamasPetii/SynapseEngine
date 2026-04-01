#ifndef SYN_INCLUDES_PUSH_CONSTANTS_POINT_LIGHT_CULLING_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_POINT_LIGHT_CULLING_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct PointLightCullingPC {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t pointLightColliderDataAddr;
    uint64_t visibleLightAddr;
    uint64_t indirectCommandAddr;
    uint     totalLightsToTest;
    uint     activeCameraEntity;
};

#endif