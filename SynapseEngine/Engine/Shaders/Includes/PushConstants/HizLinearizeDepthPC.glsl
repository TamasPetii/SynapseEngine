#ifndef SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct HizLinearizeDepthPC {
    uint64_t cameraBufferAddr; 
    uint64_t cameraSparseMapBufferAddr;  
    vec2 outImageSize;
    uint activeCameraEntity;
};

#endif