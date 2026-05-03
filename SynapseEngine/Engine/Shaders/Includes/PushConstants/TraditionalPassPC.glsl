#ifndef SYN_INCLUDES_PC_TRADITIONAL_PASS_GLSL
#define SYN_INCLUDES_PC_TRADITIONAL_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct TraditionalPassPC {
    uint64_t frameGlobalContextBufferAddr;
    uint baseDescriptorOffset;
    uint materialRenderType;
};

#endif