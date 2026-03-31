#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_SPOT_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_SPOT_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DeferredSpotLightPC {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t spotLightDataAddr;
    uint64_t spotLightSparseMapAddr;
    uint64_t spotLightColliderDataAddr;
    uint64_t visibleLightAddr;
    uint64_t vertexPositionsAddr;
    uint64_t indicesAddr;
    uint64_t spotLightShadowSparseMapAddr;
    uint64_t spotLightShadowDataAddr;  
    uint activeCameraEntity;
    float screenWidth;
    float screenHeight;
};

#endif