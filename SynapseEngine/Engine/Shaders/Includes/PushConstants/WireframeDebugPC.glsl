#ifndef SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_LIGHT_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_WIREFRAME_LIGHT_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct WireframeDebugPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t vertexPositionBufferAddr;
    uint     lightDrawType; // 0: Point Sphere, 1: Point Aabb, 2: Spot Sphere, 3: Spot Box
};

#endif