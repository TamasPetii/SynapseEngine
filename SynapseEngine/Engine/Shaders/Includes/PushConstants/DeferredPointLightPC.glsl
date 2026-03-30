#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_POINT_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_POINT_LIGHT_PC_GLSL

#include "../Core.glsl"

layout(push_constant) uniform PushConstants {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t pointLightDataAddr;
    uint64_t pointLightSparseMapAddr;
    uint64_t visibleLightAddr;
    uint64_t vertexPositionsAddr;
    uint64_t indicesAddr;
    uint64_t pointLightShadowSparseMapAddr;
    uint64_t pointLightShadowDataAddr;
    
    uint activeCameraEntity;
    float screenWidth;
    float screenHeight;
} pc;

#endif