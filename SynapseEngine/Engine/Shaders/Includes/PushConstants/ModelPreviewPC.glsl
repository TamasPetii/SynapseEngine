#ifndef SYN_INCLUDES_PC_MODEL_PREVIEW_GLSL
#define SYN_INCLUDES_PC_MODEL_PREVIEW_GLSL

#include "../SharedGpuTypes.glsl"

struct ModelPreviewPC {
    mat4 mvp;
    uint64_t frameGlobalContextBufferAddr;
    uint modelId;
    uint meshIndex;
};

#endif