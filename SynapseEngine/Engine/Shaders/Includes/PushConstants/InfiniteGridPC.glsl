#ifndef SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_HIZ_LINEARIZE_DEPTH_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct InfiniteGridPC {
    uint64_t frameGlobalContextBufferAddr;
    uint planeFlags;     // Bit 0: XZ, Bit 1: XY, Bit 2: YZ
    uint axisFlags;      // Bit 0: X,  Bit 1: Y,  Bit 2: Z
    float gridScale;
    float fadeDistance;
    float gridThickness;
    float axisThickness;
    vec4 gridColor;
    vec4 axisXColor;
    vec4 axisYColor;
    vec4 axisZColor;
};

#endif