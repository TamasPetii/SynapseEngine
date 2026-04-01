#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_DIRECTION_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_DIRECTION_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DeferredDirectionLightPC {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t directionLightDataAddr;
    uint64_t directionLightSparseMapAddr;
    uint64_t visibleLightAddr;
    uint64_t directionLightShadowSparseMapAddr;
    uint64_t directionLightShadowDataAddr;
    uint activeCameraEntity;
    float screenWidth;
    float screenHeight;
    uint _pad;
};

#endif