#ifndef SYN_INCLUDES_PUSH_CONSTANTS_SKY_SPHERE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_SKY_SPHERE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct SkySpherePC {
    mat4 skyRotationMatrix;
    uint64_t frameGlobalContextBufferAddr;
    uint skyTextureIndex;
    uint samplerIndex;
    uint mappingType;
    float skyIntensity;
};
#endif