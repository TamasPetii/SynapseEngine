#ifndef SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_EMISSIVE_AO_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_DEFERRED_EMISSIVE_AO_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct DeferredEmissiveAoPC {
    float ambientStrength;
    float emissiveStrength;
};

#endif