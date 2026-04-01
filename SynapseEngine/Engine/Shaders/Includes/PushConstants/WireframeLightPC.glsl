#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct WireframeLightPC {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t lightDataAddr;
    uint64_t lightColliderDataAddr;
    uint64_t lightSparseMapAddr;
    uint64_t visibleLightAddr;
    uint64_t vertexBufferAddr;
    uint64_t indexBufferAddr;
    uint     activeCameraEntity;
    uint     lightDrawType; // 0: Point Sphere, 1: Point Aabb, 2: Spot Sphere, 3: Spot Box
    uint     padding;
};

#endif