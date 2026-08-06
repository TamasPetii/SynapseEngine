#ifndef SYN_INCLUDES_PC_ANIMATION_PREVIEW_GLSL
#define SYN_INCLUDES_PC_ANIMATION_PREVIEW_GLSL

#include "../SharedGpuTypes.glsl"

struct AnimationPreviewPC {
    mat4 mvp;
    uint64_t frameGlobalContextBufferAddr;
    uint modelId;
    uint animationId;
    uint meshIndex;
    uint frameIndex;
};

#endif