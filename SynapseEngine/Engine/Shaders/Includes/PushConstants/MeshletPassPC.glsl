#ifndef SYN_INCLUDES_PC_MESHLET_PASS_GLSL
#define SYN_INCLUDES_PC_MESHLET_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct MeshletPassPC {
    uint64_t frameGlobalContextBufferAddr;
    uint baseDescriptorOffset;
    uint materialRenderType;
};

#endif