#ifndef SYN_INCLUDES_PUSH_CONSTANTS_BILLBOARD_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_BILLBOARD_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct BillboardPC {
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapAddr;
    uint64_t visibleEntitiesAddr;  
    uint activeCameraEntity;
    float baseScale;
};
#endif