#ifndef SYN_INCLUDES_PUSH_CONSTANTS_BILLBOARD_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_BILLBOARD_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct BillboardPC {
    uint64_t frameGlobalContextBufferAddr;
    uint64_t visibleEntitiesAddr;
    float baseScale;
};
#endif