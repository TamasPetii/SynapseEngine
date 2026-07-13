#ifndef SYN_INCLUDES_PC_MATERIAL_PREVIEW_GLSL
#define SYN_INCLUDES_PC_MATERIAL_PREVIEW_GLSL

#include "../SharedGpuTypes.glsl"

struct MaterialPreviewPC {
    uint64_t frameGlobalContextBufferAddr;
    uint materialId;
};

#endif