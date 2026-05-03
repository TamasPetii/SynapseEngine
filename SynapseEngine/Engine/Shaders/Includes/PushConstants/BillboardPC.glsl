#ifndef SYN_INCLUDES_PUSH_CONSTANTS_BILLBOARD_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_BILLBOARD_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct BillboardPC {
    uint64_t frameGlobalContextBufferAddr;
    float baseScale;
};
#endif