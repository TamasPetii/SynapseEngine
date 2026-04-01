#ifndef SYN_INCLUDES_PUSH_CONSTANTS_SPOT_LIGHT_CULLING_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_SPOT_LIGHT_CULLING_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct SpotLightCullingPC {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t spotLightColliderDataAddr;
    uint64_t visibleLightAddr;
    uint64_t indirectCommandAddr;
    uint totalLightsToTest;
    uint activeCameraEntity;
    uint enableOcclusionCulling;
    float screenWidth;
    float screenHeight;
};

#endif